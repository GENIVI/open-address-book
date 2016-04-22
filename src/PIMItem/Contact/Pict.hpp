/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Pict.hpp
 */

#ifndef PICT_PHOTO_LOGO_LIBRARY_HPP
#define PICT_PHOTO_LOGO_LIBRARY_HPP

#include <cstddef>
#include <string>

int base64decode(const char *in, size_t inLen, unsigned char *out, size_t *outLen);
int base64encode(const char *in, size_t inLen, unsigned char *out, size_t *outLen);
std::string urlDecode(const std::string & SRC);


#endif /* PICT_PHOTO_LOGO_LIBRARY_HPP */
