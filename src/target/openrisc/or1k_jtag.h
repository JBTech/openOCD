/***************************************************************************
 *   Copyright (C) 2011 Julius Baxter                                      *
 *   julius@opencores.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef OR1K_JTAG
#define OR1K_JTAG

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "helper/types.h"

struct or1k_jtag {
	struct jtag_tap *tap;
	int or1k_jtag_inited;
	int or1k_jtag_module_selected;
	struct or1k_tap_ip *tap_ip;
	struct or1k_du *du_core;
};

#endif /* OR1K_JTAG */

