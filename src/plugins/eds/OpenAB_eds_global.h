/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */

/**
 * @file OpenAB_eds_global.h
 */

#ifndef OPENAB_EDS_GLOBAL_H_
#define OPENAB_EDS_GLOBAL_H_

#include <libebook/libebook.h>
#include <libecal/libecal.h>
#include <libedata-book/libedata-book.h>
#include <libebook-contacts/libebook-contacts.h>
#include <glib2/OpenAB_glib2_global.h>

#define FREE_CONTACTS(__contacts_p) \
    if (NULL != __contacts_p) {     \
      g_slist_free_full(__contacts_p, (GDestroyNotify) g_object_unref);   \
      __contacts_p = NULL;          \
    }

#endif // OpenAB_EDS_GLOBAL_H_
