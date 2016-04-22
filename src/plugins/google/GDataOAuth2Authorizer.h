/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file GDataOAuth2Authorizer.h
 */

#ifndef GDATAOAUTH2AUTHORIZER_H_
#define GDATAOAUTH2AUTHORIZER_H_

#include <gdata/gdata.h>

G_BEGIN_DECLS

typedef enum {
  GDATA_OAUTH2_AUTHORIZER_ERROR_INVALID_REFRESH_TOKEN = 1
} GDataOAuth2AuthorizerError;

GQuark gdata_oauth2_authorizer_error_quark (void) G_GNUC_CONST;

#define GDATA_OAUTH2_AUTHORIZER_ERROR                 gdata_oauth2_authorizer_error_quark()
#define GDATA_TYPE_OAUTH2_AUTHORIZER                  (gdata_oauth2_authorizer_get_type())
#define GDATA_OAUTH2_AUTHORIZER(o)                    (G_TYPE_CHECK_INSTANCE_CAST ((o), GDATA_TYPE_OAUTH2_AUTHORIZER, GDataOAuth2Authorizer))
#define GDATA_OAUTH2_AUTHORIZER_CLASS(k)              (G_TYPE_CHECK_CLASS_CAST ((k), GDATA_TYPE_OAUTH2_AUTHORIZER, GDataOAuth2AuthorizerClass))
#define GDATA_IS_OAUTH2_AUTHORIZER(o)                 (G_TYPE_CHECK_INSTANCE_TYPE((o), GDATA_TYPE_OAUTH2_AUTHORIZER)
#define GDATA_IS_OAUTH2_AUTHORIZER_CLASS(k)           (G_TYPE_CHECK_CLASS_TYPE((k), GDATA_TYPE_OAUTH2_AUTHORIZER)
#define GDATA_CLIENT_OAUTH2_AUTHORIZER_GET_CLASS(o)   (G_TYPE_INSTANCE_GET_CLASS((o), GDATA_TYPE_OAUTH2_AUTHORIZER, GDataOAuth2AuthorizerClass))

typedef struct _GDataOAuth2AuthorizerPrivate  GDataOAuth2AuthorizerPrivate;

typedef struct {
    GObject parent;
    GDataOAuth2AuthorizerPrivate *priv;
} GDataOAuth2Authorizer;

typedef struct {
    GObjectClass parent;
} GDataOAuth2AuthorizerClass;

GType gdata_oauth2_authorizer_get_type (void) G_GNUC_CONST;

GDataOAuth2Authorizer *gdata_oauth2_authorizer_new (GType service_type);

gboolean gdata_oauth2_authorizer_authenticate (GDataOAuth2Authorizer *self,
                                               const char* client_id,
                                               const char* client_secret,
                                               const char* refresh_token,
                                               GCancellable* cancellable,
                                               GError **error);

G_END_DECLS

#endif // GDATAOAUTH2AUTHORIZER_H_
