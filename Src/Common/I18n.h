// I18n.h
//
// Copyright (c) 2008 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __i18n_h__
#define __i18n_h__

#if defined(_WIN32) || defined(__APPLE__)

#define _(szText) szText
#define N_(szText) (szText)

#else

#include <libintl.h>
// Attempt to get rid of '"_" redefined' compiler warnings.  I'm not sure the
// proper way to verify that gnome i18n support is present, so if you have a
// better idea...
#ifndef _
#define _(szText) gettext(szText)
#endif

#ifndef N_
#define N_(szText) (szText)
#endif

#endif

#endif
