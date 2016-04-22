/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */

/**
 * @file GDataOAuth2Authorizer.c
 */
#include "GDataOAuth2Authorizer.h"
#include <string.h>
#include <json-c/json.h>

typedef struct _OAuth2SecureString {
    gchar* content;
    gchar* key;
    gsize len;
} OAuth2SecureString;

void secure_free_str (gchar* str)
{
  memset (str, 0, strlen(str));
  g_free (str);
}

void secure_wipe_str (const gchar* str)
{
  memset ((gchar*)str, 0, strlen(str));
}

void oauth2_secure_string_free (OAuth2SecureString* str)
{
  g_return_if_fail (str != NULL);

  if (str->content)
  {
    memset (str->content, 0, str->len);
    g_free (str->content);
    str->content = NULL;
  }

  if (str->key)
  {
    memset (str->key, 0, str->len);
    g_free (str->key);
    str->key = NULL;
  }

  g_free (str);
}

OAuth2SecureString* oauth2_secure_string_new (const gchar* str)
{
  OAuth2SecureString* newStr = malloc (sizeof(OAuth2SecureString));
  size_t len = strlen(str);
  newStr->len = len;
  newStr->key = g_malloc (len + 1);
  newStr->content = g_malloc (len + 1);

  newStr->key[len] = '\0';
  newStr->content[len] = '\0';
  size_t i = 0;
  while ('\0' != *str)
  {
    newStr->key[i] = rand()%255;
    newStr->content[i] = *str ^ newStr->key[i];
    str++;
    i++;
  }

  return newStr;
}

gchar* oauth2_secure_string_get (OAuth2SecureString* str)
{
  size_t i, len;
  gchar* result;

  g_return_val_if_fail (str != NULL, NULL);

  len = str->len;
  result = g_malloc(len +1);
  result[len] = '\0';

  for (i = 0; i < len; i++)
  {
    result[i] = str->key[i] ^ str->content[i];
  }

  return result;
}



struct _GDataOAuth2AuthorizerPrivate {
    SoupSession *session;
    OAuth2SecureString* token;
    GMutex mutex;
    GHashTable *authorization_domains;
};

static void authorizer_init (GDataAuthorizerInterface *iface);

static void oauth2_process_request (GDataAuthorizer *self,
                                    GDataAuthorizationDomain *domain,
                                    SoupMessage *message);

static gboolean oauth2_is_authorized_for_domain (GDataAuthorizer *self,
                                                 GDataAuthorizationDomain *domain);

static void oauth2_authorizer_dispose (GObject *object);

static void oauth2_authorizer_finalize (GObject *object);

G_DEFINE_TYPE_WITH_CODE (GDataOAuth2Authorizer, gdata_oauth2_authorizer, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GDATA_TYPE_AUTHORIZER, authorizer_init))


static void
gdata_oauth2_authorizer_class_init (GDataOAuth2AuthorizerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GDataOAuth2AuthorizerPrivate));

  gobject_class->dispose = oauth2_authorizer_dispose;
  gobject_class->finalize = oauth2_authorizer_finalize;
}

static void
authorizer_init (GDataAuthorizerInterface *iface)
{
  iface->process_request = oauth2_process_request;
  iface->is_authorized_for_domain = oauth2_is_authorized_for_domain;
}

static void
gdata_oauth2_authorizer_init (GDataOAuth2Authorizer *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GDATA_TYPE_OAUTH2_AUTHORIZER, GDataOAuth2AuthorizerPrivate);

  g_mutex_init (&self->priv->mutex);
  self->priv->session = NULL;

  self->priv->authorization_domains = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, NULL);
  self->priv->token = NULL;
}

static void
oauth2_authorizer_dispose (GObject *object)
{
  GDataOAuth2AuthorizerPrivate *priv = GDATA_OAUTH2_AUTHORIZER (object)->priv;

  if (NULL != priv->session)
  {
    g_object_unref (priv->session);
    priv->session = NULL;
  }

  G_OBJECT_CLASS (gdata_oauth2_authorizer_parent_class)->dispose (object);
}

static void
oauth2_authorizer_finalize (GObject *object)
{
  GDataOAuth2AuthorizerPrivate *priv = GDATA_OAUTH2_AUTHORIZER (object)->priv;

  if (NULL != priv->token)
  {
    oauth2_secure_string_free(priv->token);
    priv->token = NULL;
  }

  g_hash_table_destroy (priv->authorization_domains);

  g_mutex_clear(&priv->mutex);
  G_OBJECT_CLASS (gdata_oauth2_authorizer_parent_class)->finalize (object);
}

static void
oauth2_process_request (GDataAuthorizer *self,
                        GDataAuthorizationDomain *domain,
                        SoupMessage *message)
{
  (void) domain;
  GDataOAuth2AuthorizerPrivate *priv = GDATA_OAUTH2_AUTHORIZER(self)->priv;

  g_mutex_lock (&priv->mutex);

  if (priv->token != NULL)
  {
    char* token = oauth2_secure_string_get(priv->token);
    soup_message_headers_replace (message->request_headers, "Authorization", token);
    secure_free_str(token);
  }

  g_mutex_unlock (&priv->mutex);
}

static gboolean
oauth2_is_authorized_for_domain (GDataAuthorizer *self, GDataAuthorizationDomain *domain)
{
  GDataOAuth2AuthorizerPrivate *priv = GDATA_OAUTH2_AUTHORIZER(self)->priv;
  const OAuth2SecureString* token;
  gpointer result;

  g_mutex_lock (&priv->mutex);
  token = priv->token;
  result = g_hash_table_lookup (priv->authorization_domains, domain);
  g_mutex_unlock (&priv->mutex);

  g_assert (result == NULL || result == domain);

  return (token != NULL && result != NULL) ? TRUE : FALSE;
}

GDataOAuth2Authorizer*
gdata_oauth2_authorizer_new (GType service_type)
{
  g_return_val_if_fail (g_type_is_a (service_type, GDATA_TYPE_SERVICE), NULL);
  GList* i;
  GList* domains = gdata_service_get_authorization_domains (service_type);
  g_return_val_if_fail (NULL != domains, NULL);

  GDataOAuth2Authorizer *authorizer;
  authorizer = GDATA_OAUTH2_AUTHORIZER (g_object_new (GDATA_TYPE_OAUTH2_AUTHORIZER, NULL));

  authorizer->priv->session = soup_session_new_with_options ("timeout", 0,
                                                             NULL);
  soup_session_add_feature_by_type (authorizer->priv->session, SOUP_TYPE_PROXY_RESOLVER_DEFAULT);

  for (i = domains; i != NULL; i = i->next)
  {
    g_return_val_if_fail (GDATA_IS_AUTHORIZATION_DOMAIN (i->data), NULL);

    g_hash_table_insert (authorizer->priv->authorization_domains,
                         g_object_ref (GDATA_AUTHORIZATION_DOMAIN (i->data)),
                         i->data);
  }

  g_list_free (domains);

  return authorizer;
}

OAuth2SecureString*
oauth2_authorizer_parse_response (const gchar* body, GError** error)
{
  char* token = NULL;
  const char* access_token = NULL;
  const char* token_type = NULL;
  OAuth2SecureString* result = NULL;

  json_object* jobj = json_tokener_parse(body);

   if (NULL == jobj)
   {
     g_set_error_literal (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR,
                         "The server returned a malformed response");
     return NULL;
   }

   json_object_object_foreach(jobj, key, val)
   {
     if (g_strcmp0 ("access_token", key) == 0)
     {
       access_token = json_object_get_string(val);
     }
     else if (g_strcmp0 ("token_type", key) == 0)
     {
       token_type = json_object_get_string(val);
     }
   }

   if (NULL == access_token || NULL == token_type)
   {
     json_object_put(jobj);
     g_set_error_literal (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR,
                        "The server returned a malformed response");
    return NULL;
   }

   token = g_strconcat (token_type, " ", access_token, NULL);
   json_object_put(jobj);
   result = oauth2_secure_string_new (token);
   secure_free_str(token);

   return result;
}

gboolean
gdata_oauth2_authorizer_authenticate (GDataOAuth2Authorizer *self,
                                      const char* client_id,
                                      const char* client_secret,
                                      const char* refresh_token,
                                      GCancellable *cancellable,
                                      GError **error)
{
  (void) cancellable;
  GDataOAuth2AuthorizerPrivate *priv = GDATA_OAUTH2_AUTHORIZER(self)->priv;
  SoupURI *uri = soup_uri_new ("https://accounts.google.com/o/oauth2/token");
  gchar *request_body = NULL;
  SoupMessage *message = NULL;
  GHashTable *parameters = NULL;
  guint status;
  OAuth2SecureString* token = NULL;

  parameters = g_hash_table_new (g_str_hash, g_str_equal);
  g_hash_table_insert (parameters, (gpointer)"client_id", (gpointer)client_id);
  g_hash_table_insert (parameters, (gpointer)"client_secret", (gpointer)client_secret);
  g_hash_table_insert (parameters, (gpointer)"refresh_token", (gpointer)refresh_token);
  g_hash_table_insert (parameters, (gpointer)"grant_type", (gpointer)"refresh_token");
  request_body = soup_form_encode_hash (parameters);

  soup_uri_set_port (uri, 443);
  message = soup_message_new_from_uri (SOUP_METHOD_POST, uri);
  soup_uri_free (uri);
  soup_message_set_request (message, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE,
                            request_body, strlen (request_body));

  g_hash_table_destroy (parameters);

  secure_wipe_str (client_secret);
  secure_wipe_str (refresh_token);

  soup_session_send_message (priv->session, message);
  status = message->status_code;

  if (SOUP_STATUS_OK !=  status)
  {
    g_set_error_literal (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_FORBIDDEN,
                         "Access was denied by the user or server");
    g_object_unref (message);

    return FALSE;
  }
  g_assert (message->response_body->data != NULL);

  token = oauth2_authorizer_parse_response(message->response_body->data, error);
  secure_wipe_str (message->response_body->data);

  g_object_unref (message);

  if (token == NULL)
  {
    return FALSE;
  }

  g_mutex_lock (&priv->mutex);
  oauth2_secure_string_free(priv->token);
  priv->token = token;
  g_mutex_unlock (&priv->mutex);

  return TRUE;
}
