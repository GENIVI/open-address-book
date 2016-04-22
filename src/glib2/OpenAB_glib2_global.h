/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file OpenAB_glib2_global.h
 */

#ifndef OpenAB_GLIB2_GLOBAL_H_
#define OPENAB_GLIB2_GLOBAL_H_

#include <glib.h>

#define GERROR_FREE(__gerror_p) \
    if (NULL != __gerror_p){    \
      g_error_free(__gerror_p); \
      __gerror_p = NULL;        \
    }

#define GERROR_MESSAGE(__gerror_p) \
  ( ( NULL != __gerror_p) ? ( __gerror_p->message ) : "" )

#endif // OpenAB_GLIB2_GLOBAL_H_
