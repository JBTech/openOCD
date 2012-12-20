/***************************************************************************
 *   Copyright (C) 2011 by Julius Baxter                                   *
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "jtag/jtag.h"
#include "register.h"
#include "algorithm.h"
#include "target.h"
#include "tdesc.h"
#include "breakpoints.h"
#include "target_type.h"
#include "or1k_jtag.h"
#include "or1k.h"

#include <helper/time_support.h>
#include "server/server.h"
#include "server/gdb_server.h"

#include "fileio.h"

struct or1k_core_reg or1k_core_reg_list_arch_info[] =
{
	[0]   = {"r0"       , 0,  GROUP0 + 1024, NULL, NULL, "group0"},
	[1]   = {"r1"       , 1,  GROUP0 + 1025, NULL, NULL, "group0"},
	[2]   = {"r2"       , 2,  GROUP0 + 1026, NULL, NULL, "group0"},
	[3]   = {"r3"       , 3,  GROUP0 + 1027, NULL, NULL, "group0"},
	[4]   = {"r4"       , 4,  GROUP0 + 1028, NULL, NULL, "group0"},
	[5]   = {"r5"       , 5,  GROUP0 + 1029, NULL, NULL, "group0"},
	[6]   = {"r6"       , 6,  GROUP0 + 1030, NULL, NULL, "group0"},
	[7]   = {"r7"       , 7,  GROUP0 + 1031, NULL, NULL, "group0"},
	[8]   = {"r8"       , 8,  GROUP0 + 1032, NULL, NULL, "group0"},
	[9]   = {"r9"       , 9,  GROUP0 + 1033, NULL, NULL, "group0"},
	[10]  = {"r10"      , 10, GROUP0 + 1034, NULL, NULL, "group0"},
	[11]  = {"r11"      , 11, GROUP0 + 1035, NULL, NULL, "group0"},
	[12]  = {"r12"      , 12, GROUP0 + 1036, NULL, NULL, "group0"},
	[13]  = {"r13"      , 13, GROUP0 + 1037, NULL, NULL, "group0"},
	[14]  = {"r14"      , 14, GROUP0 + 1038, NULL, NULL, "group0"},
	[15]  = {"r15"      , 15, GROUP0 + 1039, NULL, NULL, "group0"},
	[16]  = {"r16"      , 16, GROUP0 + 1040, NULL, NULL, "group0"},
	[17]  = {"r17"      , 17, GROUP0 + 1041, NULL, NULL, "group0"},
	[18]  = {"r18"      , 18, GROUP0 + 1042, NULL, NULL, "group0"},
	[19]  = {"r19"      , 19, GROUP0 + 1043, NULL, NULL, "group0"},
	[20]  = {"r20"      , 20, GROUP0 + 1044, NULL, NULL, "group0"},
	[21]  = {"r21"      , 21, GROUP0 + 1045, NULL, NULL, "group0"},
	[22]  = {"r22"      , 22, GROUP0 + 1046, NULL, NULL, "group0"},
	[23]  = {"r23"      , 23, GROUP0 + 1047, NULL, NULL, "group0"},
	[24]  = {"r24"      , 24, GROUP0 + 1048, NULL, NULL, "group0"},
	[25]  = {"r25"      , 25, GROUP0 + 1049, NULL, NULL, "group0"},
	[26]  = {"r26"      , 26, GROUP0 + 1050, NULL, NULL, "group0"},
	[27]  = {"r27"      , 27, GROUP0 + 1051, NULL, NULL, "group0"},
	[28]  = {"r28"      , 28, GROUP0 + 1052, NULL, NULL, "group0"},
	[29]  = {"r29"      , 29, GROUP0 + 1053, NULL, NULL, "group0"},
	[30]  = {"r30"      , 30, GROUP0 + 1054, NULL, NULL, "group0"},
	[31]  = {"r31"      , 31, GROUP0 + 1055, NULL, NULL, "group0"},
	[32]  = {"ppc"      , 32, GROUP0 + 18,   NULL, NULL, "group0"},
	[33]  = {"npc"      , 33, GROUP0 + 16,   NULL, NULL, "group0"},
	[34]  = {"sr"       , 34, GROUP0 + 17,   NULL, NULL, "group0"},
	[35]  = {"vr"       , 35, GROUP0 + 0,    NULL, NULL, "group0"},
	[36]  = {"upr"      , 36, GROUP0 + 1,    NULL, NULL, "group0"},
	[37]  = {"cpucfgr"  , 37, GROUP0 + 2,    NULL, NULL, "group0"},
	[38]  = {"dmmucfgr" , 38, GROUP0 + 3,    NULL, NULL, "group0"},
	[39]  = {"immucfgr" , 39, GROUP0 + 4,    NULL, NULL, "group0"},
	[40]  = {"dccfgr"   , 40, GROUP0 + 5,    NULL, NULL, "group0"},
	[41]  = {"iccfgr"   , 41, GROUP0 + 6,    NULL, NULL, "group0"},
	[42]  = {"dcfgr"    , 42, GROUP0 + 7,    NULL, NULL, "group0"},
	[43]  = {"pccfgr"   , 43, GROUP0 + 8,    NULL, NULL, "group0"},
	[44]  = {"fpcsr"    , 44, GROUP0 + 20,   NULL, NULL, "group0"},
	[45]  = {"epcr0"    , 45, GROUP0 + 32,   NULL, NULL, "group0"},
	[46]  = {"epcr1"    , 46, GROUP0 + 33,   NULL, NULL, "group0"},
	[47]  = {"epcr2"    , 47, GROUP0 + 34,   NULL, NULL, "group0"},
	[48]  = {"epcr3"    , 48, GROUP0 + 35,   NULL, NULL, "group0"},
	[49]  = {"epcr4"    , 49, GROUP0 + 36,   NULL, NULL, "group0"},
	[50]  = {"epcr5"    , 50, GROUP0 + 37,   NULL, NULL, "group0"},
	[51]  = {"epcr6"    , 51, GROUP0 + 38,   NULL, NULL, "group0"},
	[52]  = {"epcr7"    , 52, GROUP0 + 39,   NULL, NULL, "group0"},
	[53]  = {"epcr8"    , 53, GROUP0 + 40,   NULL, NULL, "group0"},
	[54]  = {"epcr9"    , 54, GROUP0 + 41,   NULL, NULL, "group0"},
	[55]  = {"epcr10"   , 55, GROUP0 + 42,   NULL, NULL, "group0"},
	[56]  = {"epcr11"   , 56, GROUP0 + 43,   NULL, NULL, "group0"},
	[57]  = {"epcr12"   , 57, GROUP0 + 44,   NULL, NULL, "group0"},
	[58]  = {"epcr13"   , 58, GROUP0 + 45,   NULL, NULL, "group0"},
	[59]  = {"epcr14"   , 59, GROUP0 + 46,   NULL, NULL, "group0"},
	[60]  = {"epcr15"   , 60, GROUP0 + 47,   NULL, NULL, "group0"},
	[61]  = {"eear0"    , 61, GROUP0 + 48,   NULL, NULL, "group0"},
	[62]  = {"eear1"    , 62, GROUP0 + 49,   NULL, NULL, "group0"},
	[63]  = {"eear2"    , 63, GROUP0 + 50,   NULL, NULL, "group0"},
	[64]  = {"eear3"    , 64, GROUP0 + 51,   NULL, NULL, "group0"},
	[65]  = {"eear4"    , 65, GROUP0 + 52,   NULL, NULL, "group0"},
	[66]  = {"eear5"    , 66, GROUP0 + 53,   NULL, NULL, "group0"},
	[67]  = {"eear6"    , 67, GROUP0 + 54,   NULL, NULL, "group0"},
	[68]  = {"eear7"    , 68, GROUP0 + 55,   NULL, NULL, "group0"},
	[69]  = {"eear8"    , 69, GROUP0 + 56,   NULL, NULL, "group0"},
	[70]  = {"eear9"    , 70, GROUP0 + 57,   NULL, NULL, "group0"},
	[71]  = {"eear10"   , 71, GROUP0 + 58,   NULL, NULL, "group0"},
	[72]  = {"eear11"   , 72, GROUP0 + 59,   NULL, NULL, "group0"},
	[73]  = {"eear12"   , 73, GROUP0 + 60,   NULL, NULL, "group0"},
	[74]  = {"eear13"   , 74, GROUP0 + 61,   NULL, NULL, "group0"},
	[75]  = {"eear14"   , 75, GROUP0 + 62,   NULL, NULL, "group0"},
	[76]  = {"eear15"   , 76, GROUP0 + 63,   NULL, NULL, "group0"},
	[77]  = {"esr0"     , 77, GROUP0 + 64,   NULL, NULL, "group0"},
	[78]  = {"esr1"     , 78, GROUP0 + 65,   NULL, NULL, "group0"},
	[79]  = {"esr2"     , 79, GROUP0 + 66,   NULL, NULL, "group0"},
	[80]  = {"esr3"     , 80, GROUP0 + 67,   NULL, NULL, "group0"},
	[81]  = {"esr4"     , 81, GROUP0 + 68,   NULL, NULL, "group0"},
	[82]  = {"esr5"     , 82, GROUP0 + 69,   NULL, NULL, "group0"},
	[83]  = {"esr6"     , 83, GROUP0 + 70,   NULL, NULL, "group0"},
	[84]  = {"esr7"     , 84, GROUP0 + 71,   NULL, NULL, "group0"},
	[85]  = {"esr8"     , 85, GROUP0 + 72,   NULL, NULL, "group0"},
	[86]  = {"esr9"     , 86, GROUP0 + 73,   NULL, NULL, "group0"},
	[87]  = {"esr10"    , 87, GROUP0 + 74,   NULL, NULL, "group0"},
	[88]  = {"esr11"    , 88, GROUP0 + 75,   NULL, NULL, "group0"},
	[89]  = {"esr12"    , 89, GROUP0 + 76,   NULL, NULL, "group0"},
	[90]  = {"esr13"    , 90, GROUP0 + 77,   NULL, NULL, "group0"},
	[91]  = {"esr14"    , 91, GROUP0 + 78,   NULL, NULL, "group0"},
	[92]  = {"esr15"    , 92, GROUP0 + 79,   NULL, NULL, "group0"},

	[93]   = {"dmmuucr" , 93,  GROUP1 + 0,   NULL, NULL, "group1"},
	[94]   = {"dmmuupr" , 94,  GROUP1 + 1,   NULL, NULL, "group1"},
	[95]   = {"dtlbeir" , 95,  GROUP1 + 2,   NULL, NULL, "group1"},
	[96]   = {"datbmr0" , 96,  GROUP1 + 4,   NULL, NULL, "group1"},
	[97]   = {"datbmr1" , 97,  GROUP1 + 5,   NULL, NULL, "group1"},
	[98]   = {"datbmr2" , 98,  GROUP1 + 6,   NULL, NULL, "group1"},
	[99]   = {"datbmr3" , 99,  GROUP1 + 7,   NULL, NULL, "group1"},
	[100]  = {"datbtr0" , 100, GROUP1 + 8,   NULL, NULL, "group1"},
	[101]  = {"datbtr1" , 101, GROUP1 + 9,   NULL, NULL, "group1"},
	[102]  = {"datbtr2" , 102, GROUP1 + 10,  NULL, NULL, "group1"},
	[103]  = {"datbtr3" , 103, GROUP1 + 11,  NULL, NULL, "group1"},
/*
	[104..231]   dtlbw0mr0 - 127, GROUP1 512 - 639
	[232..359]   dtlbw0tr0 - 127, GROUP1 640 - 767
	[360..487]   dtlbw1mr0 - 127, GROUP1 768 - 895
	[488..615]   dtlbw1tr0 - 127, GROUP1 896 - 1023
	[616..743]   dtlbw2mr0 - 127, GROUP1 1024 - 1151
	[744..871]   dtlbw2tr0 - 127, GROUP1 1152 - 1279
	[872..999]   dtlbw3mr0 - 127, GROUP1 1280 - 1407
	[1000..1127] dtlbw3tr0 - 127, GROUP1 1408 - 1535
*/
	[1128] = {"immucr"  , 1128, GROUP2 + 0,  NULL, NULL, "group2"},
	[1129] = {"immupr"  , 1129, GROUP2 + 1,  NULL, NULL, "group2"},
	[1130] = {"itlbeir" , 1130, GROUP2 + 2,  NULL, NULL, "group2"},
	[1131] = {"iatbmr0" , 1131, GROUP2 + 4,  NULL, NULL, "group2"},
	[1132] = {"iatbmr1" , 1132, GROUP2 + 5,  NULL, NULL, "group2"},
	[1133] = {"iatbmr2" , 1133, GROUP2 + 6,  NULL, NULL, "group2"},
	[1134] = {"iatbmr3" , 1134, GROUP2 + 7,  NULL, NULL, "group2"},
	[1135] = {"iatbtr0" , 1135, GROUP2 + 8,  NULL, NULL, "group2"},
	[1136] = {"iatbtr1" , 1136, GROUP2 + 9,  NULL, NULL, "group2"},
	[1137] = {"iatbtr2" , 1137, GROUP2 + 10, NULL, NULL, "group2"},
	[1138] = {"iatbtr3" , 1138, GROUP2 + 11, NULL, NULL, "group2"},
/*
	[1139..1266] itlbw0mr0 - 127, GROUP2 512 - 639
	[1267..1394] itlbw0tr0 - 127, GROUP2 640 - 767
	[1395..1522] itlbw1mr0 - 127, GROUP2 768 - 895
	[1523..1650] itlbw1tr0 - 127, GROUP2 896 - 1023
	[1651..1778] itlbw2mr0 - 127, GROUP2 1024 - 1151
	[1779..1906] itlbw2tr0 - 127, GROUP2 1152 - 1279
	[1907..2034] itlbw3mr0 - 127, GROUP2 1280 - 1407
	[2035..2162] itlbw3tr0 - 127, GROUP2 1408 - 1535
*/
	[2163] = {"dccr"    , 2163, GROUP3 + 0,  NULL, NULL, "group3"},
	[2164] = {"dcbpr"   , 2164, GROUP3 + 1,  NULL, NULL, "group3"},
	[2165] = {"dcbfr"   , 2165, GROUP3 + 2,  NULL, NULL, "group3"},
	[2166] = {"dcbir"   , 2166, GROUP3 + 3,  NULL, NULL, "group3"},
	[2167] = {"dcbwr"   , 2167, GROUP3 + 4,  NULL, NULL, "group3"},
	[2168] = {"dcblr"   , 2168, GROUP3 + 5,  NULL, NULL, "group3"},

	[2169] = {"iccr"    , 2169, GROUP4 + 0,  NULL, NULL, "group4"},
	[2170] = {"icbpr"   , 2170, GROUP4 + 1,  NULL, NULL, "group4"},
	[2171] = {"icbir"   , 2171, GROUP4 + 2,  NULL, NULL, "group4"},
	[2172] = {"icblr"   , 2172, GROUP4 + 3,  NULL, NULL, "group4"},

	[2173] = {"maclo"   , 2173, GROUP5 + 0,  NULL, NULL, "group5"},
	[2174] = {"machi"   , 2174, GROUP5 + 1,  NULL, NULL, "group5"},

	[2175] = {"dvr0"    , 2175, GROUP6 + 0,  NULL, NULL, "group6"},
	[2176] = {"dvr1"    , 2176, GROUP6 + 1,  NULL, NULL, "group6"},
	[2177] = {"dvr2"    , 2177, GROUP6 + 2,  NULL, NULL, "group6"},
	[2178] = {"dvr3"    , 2178, GROUP6 + 3,  NULL, NULL, "group6"},
	[2179] = {"dvr4"    , 2179, GROUP6 + 4,  NULL, NULL, "group6"},
	[2180] = {"dvr5"    , 2180, GROUP6 + 5,  NULL, NULL, "group6"},
	[2181] = {"dvr6"    , 2181, GROUP6 + 6,  NULL, NULL, "group6"},
	[2182] = {"dvr7"    , 2182, GROUP6 + 7,  NULL, NULL, "group6"},
	[2183] = {"dcr0"    , 2183, GROUP6 + 8,  NULL, NULL, "group6"},
	[2184] = {"dcr1"    , 2184, GROUP6 + 9,  NULL, NULL, "group6"},
	[2185] = {"dcr2"    , 2185, GROUP6 + 10, NULL, NULL, "group6"},
	[2186] = {"dcr3"    , 2186, GROUP6 + 11, NULL, NULL, "group6"},
	[2187] = {"dcr4"    , 2187, GROUP6 + 12, NULL, NULL, "group6"},
	[2188] = {"dcr5"    , 2188, GROUP6 + 13, NULL, NULL, "group6"},
	[2189] = {"dcr6"    , 2189, GROUP6 + 14, NULL, NULL, "group6"},
	[2190] = {"dcr7"    , 2190, GROUP6 + 15, NULL, NULL, "group6"},
	[2191] = {"dmr1"    , 2191, GROUP6 + 16, NULL, NULL, "group6"},
	[2192] = {"dmr2"    , 2192, GROUP6 + 17, NULL, NULL, "group6"},
	[2193] = {"dcwr0"   , 2193, GROUP6 + 18, NULL, NULL, "group6"},
	[2194] = {"dcwr1"   , 2194, GROUP6 + 19, NULL, NULL, "group6"},
	[2195] = {"dsr"     , 2195, GROUP6 + 20, NULL, NULL, "group6"},
	[2196] = {"drr"     , 2196, GROUP6 + 21, NULL, NULL, "group6"},

	[2197] = {"pccr0"   , 2197, GROUP7 + 0,  NULL, NULL, "group7"},
	[2198] = {"pccr1"   , 2198, GROUP7 + 1,  NULL, NULL, "group7"},
	[2199] = {"pccr2"   , 2199, GROUP7 + 2,  NULL, NULL, "group7"},
	[2200] = {"pccr3"   , 2200, GROUP7 + 3,  NULL, NULL, "group7"},
	[2201] = {"pccr4"   , 2201, GROUP7 + 4,  NULL, NULL, "group7"},
	[2202] = {"pccr5"   , 2202, GROUP7 + 5,  NULL, NULL, "group7"},
	[2203] = {"pccr6"   , 2203, GROUP7 + 6,  NULL, NULL, "group7"},
	[2204] = {"pccr7"   , 2204, GROUP7 + 7,  NULL, NULL, "group7"},
	[2205] = {"pcmr0"   , 2205, GROUP7 + 8,  NULL, NULL, "group7"},
	[2206] = {"pcmr1"   , 2206, GROUP7 + 9,  NULL, NULL, "group7"},
	[2207] = {"pcmr2"   , 2207, GROUP7 + 10, NULL, NULL, "group7"},
	[2208] = {"pcmr3"   , 2208, GROUP7 + 11, NULL, NULL, "group7"},
	[2209] = {"pcmr4"   , 2209, GROUP7 + 12, NULL, NULL, "group7"},
	[2210] = {"pcmr5"   , 2210, GROUP7 + 13, NULL, NULL, "group7"},
	[2211] = {"pcmr6"   , 2211, GROUP7 + 14, NULL, NULL, "group7"},
	[2212] = {"pcmr7"   , 2212, GROUP7 + 15, NULL, NULL, "group7"},

	[2213] = {"pmr"     , 2213, GROUP8 + 0,  NULL, NULL, "group8"},

	[2214] = {"picmr"   , 2214, GROUP9 + 0,  NULL, NULL, "pic"},
	[2215] = {"picsr"   , 2215, GROUP9 + 2,  NULL, NULL, ""},

	[2216] = {"ttmr"    , 2216, GROUP10 + 0, NULL, NULL, "timer"},
	[2217] = {"ttcr"    , 2217, GROUP10 + 1, NULL, NULL, NULL},
};


static int or1k_read_core_reg(struct target *target, int num);
static int or1k_write_core_reg(struct target *target, int num);

static int or1k_jtag_read_regs(struct or1k_jtag *jtag_info, uint32_t *regs)
{
	or1k_jtag_read_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_R0].spr_num, OR1K_REG_R31+1,
			regs+OR1K_REG_R0);

	or1k_jtag_read_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_PPC].spr_num, 1,
			regs+OR1K_REG_PPC);

	or1k_jtag_read_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_NPC].spr_num, 1,
			regs+OR1K_REG_NPC);

	or1k_jtag_read_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_SR].spr_num, 1,
			regs+OR1K_REG_SR);

	return ERROR_OK;
}

static int or1k_jtag_write_regs(struct or1k_jtag *jtag_info, uint32_t *regs)
{
	or1k_jtag_write_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_R0].spr_num, OR1K_REG_R31+1,
			&regs[OR1K_REG_R0]);

	or1k_jtag_write_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_PPC].spr_num, 1,
			&regs[OR1K_REG_PPC]);

	or1k_jtag_write_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_NPC].spr_num, 1,
			&regs[OR1K_REG_NPC]);

	or1k_jtag_write_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_SR].spr_num, 1,
			&regs[OR1K_REG_SR]);

	return ERROR_OK;
}

int or1k_save_context(struct target *target)
{

	LOG_DEBUG(" - ");
	int retval, i, regs_read = 0;
	struct or1k_common *or1k = target_to_or1k(target);
	
	for (i = 0; i < OR1KNUMCOREREGS; i++)
	{
		if (!regs_read && !or1k->core_cache->reg_list[i].valid)
		{
			/* read all registers at once (but only one time in this loop) */
			retval = or1k_jtag_read_regs(&or1k->jtag, or1k->core_regs);
			if (retval != ERROR_OK)
				return retval;
			
			/* prevent next reads in this loop */
			regs_read = 1;
		}
		if (!or1k->core_cache->reg_list[i].valid) {
			/* We've just updated the core_reg[i], now update
			   the core cache */
			or1k_read_core_reg(target, i);
		}
	}

	return ERROR_OK;
}

int or1k_restore_context(struct target *target)
{
	int i;

	LOG_DEBUG(" - ");

	/* get pointers to arch-specific information */
	struct or1k_common *or1k = target_to_or1k(target);

	for (i = 0; i < OR1KNUMCOREREGS; i++)
	{
		if (or1k->core_cache->reg_list[i].dirty)
		{
			or1k_write_core_reg(target, i);
		}
	}

	/* write core regs */
	or1k_jtag_write_regs(&or1k->jtag, or1k->core_regs);

	return ERROR_OK;
}

static int or1k_read_core_reg(struct target *target, int num)
{
	uint32_t reg_value;
	int retval;

	/* get pointers to arch-specific information */
	struct or1k_common *or1k = target_to_or1k(target);

	if ((num < 0) || (num >= NBR_DEFINED_REGISTERS))
		return ERROR_COMMAND_SYNTAX_ERROR;

	if ((num >= 0) && (num < OR1KNUMCOREREGS)) {
		reg_value = or1k->core_regs[num];
		buf_set_u32(or1k->core_cache->reg_list[num].value, 0, 32, reg_value);
		LOG_DEBUG("read core reg %i value 0x%" PRIx32 "", num , reg_value);
		or1k->core_cache->reg_list[num].valid = 1;
		or1k->core_cache->reg_list[num].dirty = 0;
	} else {
		/* This is an spr, always read value from HW */
		retval = or1k_jtag_read_cpu(&or1k->jtag, or1k_core_reg_list_arch_info[num].spr_num, 1, &reg_value);
		if (retval != ERROR_OK)
			return retval;
		buf_set_u32(or1k->core_cache->reg_list[num].value, 0, 32, reg_value);
		LOG_DEBUG("read spr reg %i value 0x%" PRIx32 "", num , reg_value);
	}

	return ERROR_OK;
}

static int or1k_write_core_reg(struct target *target, int num)
{
	uint32_t reg_value;

	/* get pointers to arch-specific information */
	struct or1k_common *or1k = target_to_or1k(target);

	if ((num < 0) || (num >= OR1KNUMCOREREGS))
		return ERROR_COMMAND_SYNTAX_ERROR;

	reg_value = buf_get_u32(or1k->core_cache->reg_list[num].value, 0, 32);
	or1k->core_regs[num] = reg_value;
	LOG_DEBUG("write core reg %i value 0x%" PRIx32 "", num , reg_value);
	or1k->core_cache->reg_list[num].valid = 1;
	or1k->core_cache->reg_list[num].dirty = 0;

	return ERROR_OK;
}

static int or1k_get_core_reg(struct reg *reg)
{
	int retval;
	struct or1k_core_reg *or1k_reg = reg->arch_info;
	struct target *target = or1k_reg->target;

	if (target->state != TARGET_HALTED)
	{
		return ERROR_TARGET_NOT_HALTED;
	}

	retval = or1k_read_core_reg(target, or1k_reg->list_num);

	return retval;
}

static int or1k_set_core_reg(struct reg *reg, uint8_t *buf)
{
	struct or1k_core_reg *or1k_reg = reg->arch_info;
	struct target *target = or1k_reg->target;
	struct or1k_common *or1k = target_to_or1k(target);
	uint32_t value = buf_get_u32(buf, 0, 32);
	int retval;

	if (target->state != TARGET_HALTED)
	{
		return ERROR_TARGET_NOT_HALTED;
	}

	if (or1k_reg->list_num < OR1KNUMCOREREGS) {
		buf_set_u32(reg->value, 0, 32, value);
		reg->dirty = 1;
		reg->valid = 1;
	} else {
		/* This is an spr, write it to the HW */
		retval = or1k_jtag_write_cpu(&or1k->jtag, or1k_reg->spr_num, 1, &value);
		if (retval != ERROR_OK)
			return retval;
	}

	return ERROR_OK;
}

static const struct reg_arch_type or1k_reg_type = {
	.get = or1k_get_core_reg,
	.set = or1k_set_core_reg,
};

static struct reg_cache *or1k_build_reg_cache(struct target *target)
{
	int num_regs = OR1KNUMCOREREGS;
	struct or1k_common *or1k = target_to_or1k(target);
	struct reg_cache **cache_p = register_get_last_cache_p(&target->reg_cache);
	struct reg_cache *cache = malloc(sizeof(struct reg_cache));
	struct reg *reg_list = malloc(sizeof(struct reg) * (NBR_DEFINED_REGISTERS));
	struct or1k_core_reg *arch_info = 
		malloc(sizeof(struct or1k_core_reg) * NBR_DEFINED_REGISTERS);
	int i;

	/* Build the process context cache */
	cache->name = "openrisc 1000 registers";
	cache->next = NULL;
	cache->reg_list = reg_list;
	cache->num_regs = num_regs;
	(*cache_p) = cache;
	or1k->core_cache = cache;

	for (i = 0; i < NBR_DEFINED_REGISTERS; i++)
	{
		arch_info[i] = or1k_core_reg_list_arch_info[i];
		arch_info[i].target = target;
		arch_info[i].or1k_common = or1k;
		reg_list[i].name = or1k_core_reg_list_arch_info[i].name;
		reg_list[i].feature = or1k_core_reg_list_arch_info[i].feature;
		reg_list[i].size = 32;
		reg_list[i].value = calloc(1, 4);
		reg_list[i].dirty = 0;
		reg_list[i].valid = 0;
		reg_list[i].type = &or1k_reg_type;
		reg_list[i].arch_info = &arch_info[i];
	}

	return cache;
}

static int or1k_generate_tdesc(struct target *target, const char *filename)
{
	struct fileio fileio;
	char **features = NULL;
	int current_feature = 0;
	int retval;

	/* Create the tdesc file, prepare the header and set the architecture
	 * to or32 in the file.
	 */
	retval = open_and_init_tdesc_file(&fileio, filename, "or32");
	if (retval != ERROR_OK)
		return retval;

	/* Get a list of available target registers features */
	retval = get_reg_features_list(target, &features);
	if (retval < 0)
		return ERROR_FAIL;

	/* If we found some features associated with registers, create sections */
	if (features != NULL) {
		while (features[current_feature]) {
			generate_feature_section(target, &fileio, features[current_feature]);
			current_feature++;
		}
	}

	/* For registers without features, put them in "nogroup" feature */
	if (count_reg_without_group(target) > 0) {
		generate_feature_section(target, &fileio, NULL);
	}

	free(features);
	close_tdesc_file(&fileio);

	return ERROR_OK;
}

static int or1k_debug_entry(struct target *target)
{

  	/* Perhaps do more debugging entry (processor stalled) set up here */

	LOG_DEBUG(" - ");

	or1k_save_context(target);

	return ERROR_OK;
}

static int or1k_halt(struct target *target)
{
	struct or1k_common *or1k = target_to_or1k(target);

	LOG_DEBUG("target->state: %s",
		  target_state_name(target));

	if (target->state == TARGET_HALTED)
	{
		LOG_DEBUG("target was already halted");
		return ERROR_OK;
	}

	if (target->state == TARGET_UNKNOWN)
	{
		LOG_WARNING("target was in unknown state when halt was requested");
	}

	if (target->state == TARGET_RESET)
	{
		if ((jtag_get_reset_config() & RESET_SRST_PULLS_TRST) && 
		    jtag_get_srst())
		{
			LOG_ERROR("can't request a halt while in reset if nSRST pulls nTRST");
			return ERROR_TARGET_FAILURE;
		}
		else
		{
			target->debug_reason = DBG_REASON_DBGRQ;

			return ERROR_OK;
		}
	}

	/* Mohor debug unit-specific. */
	or1k_jtag_write_cpu_cr(&or1k->jtag, 1, 0);

	target->debug_reason = DBG_REASON_DBGRQ;

	return ERROR_OK;
}

static int or1k_is_cpu_running(struct target *target, int* running)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int retval;
	uint32_t cpu_cr;

	/* Have a retry loop to determine of the CPU is running.
	   If target has been hard reset for any reason, it might take a couple
	   of goes before it's ready again.
	*/

	int tries = 0;
	const int RETRIES_MAX = 5;
	while (tries < RETRIES_MAX)
	{

		tries++;
		/* Specific to Mohor debug interface - others may have to do
		 * something different here. 
		 */
		retval = or1k_jtag_read_cpu_cr(&or1k->jtag, &cpu_cr);

		if (retval != ERROR_OK)
		{
			LOG_WARNING("Debug IF CPU control reg read failure.");
			/* Try once to restart the JTAG infrastructure -
			   quite possibly the board has just been reset. */
			LOG_WARNING("Resetting JTAG TAP state and reconnectiong to debug IF.");
			or1k_jtag_init(&or1k->jtag);
			
			LOG_WARNING("attempt %d of %d",tries,RETRIES_MAX);

			/* TODO: perhaps some sort of time delay here. 1s? */
			sleep(1);
				
			

			continue;
			
		}
		else
		{
			/* Specific to Mohor debug interface */
			*running = !(cpu_cr & OR1K_MOHORDBGIF_CPU_CR_STALL);
			return retval;
		}
	}
	LOG_WARNING("Could not re-establish communication with target");
	return retval;
}

static int or1k_poll(struct target *target)
{
	
	int retval;
	int running;

	retval = or1k_is_cpu_running(target, &running);
	if (retval != ERROR_OK)
		return retval;

	/*LOG_DEBUG("running: %d",running);*/
	
	
	/* check for processor halted */
	if (!running)
	{
		/* It's actually stalled, so update our software's state */
		if ((target->state == TARGET_RUNNING) || 
		    (target->state == TARGET_RESET))
		{
			target->state = TARGET_HALTED;

			if ((retval = or1k_debug_entry(target)) != ERROR_OK)
				return retval;

			target_call_event_callbacks(target, 
						    TARGET_EVENT_HALTED);
		}
		else if (target->state == TARGET_DEBUG_RUNNING)
		{
			target->state = TARGET_HALTED;

			if ((retval = or1k_debug_entry(target)) != ERROR_OK)
				return retval;

			target_call_event_callbacks(target, 
						    TARGET_EVENT_DEBUG_HALTED);
		}
	}
	else /* ... target is running */
	{
		
		/* If target was supposed to be stalled, stall it again */
		if  (target->state == TARGET_HALTED)
		{

			target->state = TARGET_RUNNING;
			
			or1k_halt(target);

			if ((retval = or1k_debug_entry(target)) != ERROR_OK)
				return retval;

			target_call_event_callbacks(target, 
						    TARGET_EVENT_DEBUG_HALTED);
		}
		

		target->state = TARGET_RUNNING;

	}


	return ERROR_OK;
}

static int or1k_assert_reset(struct target *target)
{
	LOG_ERROR("%s: implement me", __func__);

	return ERROR_OK;
}

static int or1k_deassert_reset(struct target *target)
{
	LOG_ERROR("%s: implement me", __func__);

	return ERROR_OK;
}

static int or1k_soft_reset_halt(struct target *target)
{
	LOG_ERROR("%s: implement me", __func__);

	return ERROR_OK;
}

static int or1k_resume_or_step(struct target *target, int current,
			       uint32_t address, int handle_breakpoints, 
			       int debug_execution, int step)
{
	struct or1k_common *or1k = target_to_or1k(target);
	struct breakpoint *breakpoint = NULL;
	uint32_t resume_pc;
	int retval;

	LOG_DEBUG(" - ");
	LOG_DEBUG(" addr: 0x%x, stepping: %d, handle breakpoints %d\n",
		  address, step, handle_breakpoints);

	if (target->state != TARGET_HALTED)
	{
		LOG_WARNING("target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	if (!debug_execution)
	{
		target_free_all_working_areas(target);
	}

	/* current ? continue on current pc : continue at <address> */
	if (!current)
	{
		buf_set_u32(or1k->core_cache->reg_list[OR1K_REG_NPC].value, 0,
			    32, address);
	}

	if (!step) {
		or1k_restore_context(target);
	}

	uint32_t debug_reg_list[OR1K_DEBUG_REG_NUM];
	/* read debug registers (starting from DMR1 register) */
	or1k_jtag_read_cpu(&or1k->jtag, OR1K_DMR1_CPU_REG_ADD, OR1K_DEBUG_REG_NUM, debug_reg_list);
	/* Clear Debug Reason Register (DRR) */
	debug_reg_list[OR1K_DEBUG_REG_DRR] = 0;
	/* Clear watchpoint break generation in Debug Mode Register 2 (DMR2) */
	debug_reg_list[OR1K_DEBUG_REG_DMR2] &= ~OR1K_DMR2_WGB;
	if (step)
		/* Set the single step trigger in Debug Mode Register 1 (DMR1) */
		debug_reg_list[OR1K_DEBUG_REG_DMR1] |= OR1K_DMR1_ST | OR1K_DMR1_BT;
	else
		/* Clear the single step trigger in Debug Mode Register 1 (DMR1) */
		debug_reg_list[OR1K_DEBUG_REG_DMR1] &= ~(OR1K_DMR1_ST | OR1K_DMR1_BT);

	/* Set traps to be handled by the debug unit in the Debug Stop 
	   Register (DSR) */
	/* TODO - check if we have any software breakpoints in place before
	   setting this value - the kernel, for instance, relies on l.trap
	   instructions not stalling the processor! */
	debug_reg_list[OR1K_DEBUG_REG_DSR] |= OR1K_DSR_TE;
	/* write debug registers (starting from DMR1 register) */
	or1k_jtag_write_cpu(&or1k->jtag, OR1K_DMR1_CPU_REG_ADD, OR1K_DEBUG_REG_NUM, debug_reg_list);

	resume_pc = buf_get_u32(or1k->core_cache->reg_list[OR1K_REG_NPC].value,
				0, 32);
	/* the front-end may request us not to handle breakpoints */
	if (handle_breakpoints)
	{
		/* Single step past breakpoint at current address */
		if ((breakpoint = breakpoint_find(target, resume_pc)))
		{
			LOG_DEBUG("unset breakpoint at 0x%8.8" PRIx32 "", breakpoint->address);
#if 0
			/* Do appropriate things here to remove breakpoint. */
#endif
		}
	}
	/* Unstall time */

	/* Mohor debug if, clearing control register unstalls */
	retval = or1k_jtag_write_cpu_cr(&or1k->jtag, 0, 0);
	if (retval != ERROR_OK)
		return retval;

	if (step)
		target->debug_reason = DBG_REASON_SINGLESTEP;
	else
		target->debug_reason = DBG_REASON_NOTHALTED;

	/* registers are now invalid */
	register_cache_invalidate(or1k->core_cache);

	if (!debug_execution)
	{
		target->state = TARGET_RUNNING;
		target_call_event_callbacks(target, TARGET_EVENT_RESUMED);
		LOG_DEBUG("target resumed at 0x%" PRIx32 "", resume_pc);
	}
	else
	{
		target->state = TARGET_DEBUG_RUNNING;
		target_call_event_callbacks(target, TARGET_EVENT_DEBUG_RESUMED);
		LOG_DEBUG("target debug resumed at 0x%" PRIx32 "", resume_pc);
	}
	
	return ERROR_OK;
}

static int or1k_resume(struct target *target, int current,
		uint32_t address, int handle_breakpoints, int debug_execution)
{
	return or1k_resume_or_step(target, current, address, 
				   handle_breakpoints, 
				   debug_execution, 
				   /* Single step? No. */
				   0);
}

static int or1k_step(struct target *target, int current,
		     uint32_t address, int handle_breakpoints)
{
	return or1k_resume_or_step(target, current, address, 
				   handle_breakpoints, 
				   /* TARGET_EVENT_RESUMED:
				      target resumed to execute user code */
				   0,
				   /* Single step? Yes. */
				   1);
	
}

static int or1k_add_breakpoint(struct target *target, 
			       struct breakpoint *breakpoint)
{
	LOG_DEBUG("Adding breakpoint: addr %08x, len %d, type %d, set: %d, id: %d",
		  breakpoint->address, breakpoint->length, breakpoint->type,
		  breakpoint->set, breakpoint->unique_id);

	struct or1k_common *or1k = target_to_or1k(target);

	/* Only support SW breakpoints for now. */
	if (breakpoint->type == BKPT_HARD)
		LOG_ERROR("HW breakpoints not supported for now. Doing SW breakpoint.");
	
	/* Read and save the instruction */
	or1k_jtag_read_memory32(&or1k->jtag, 
				breakpoint->address , 
				1,
				(uint32_t*)breakpoint->orig_instr);

	/* Sub in the OR1K trap instruction */
	uint32_t or1k_trap_insn;
	/* set correct endianess */
	target_buffer_set_u32(target, (uint8_t*)&or1k_trap_insn, OR1K_TRAP_INSTR);
	or1k_jtag_write_memory32(&or1k->jtag, 
				breakpoint->address , 
				 1,
				 (uint32_t*)&or1k_trap_insn);

	/* invalidate instruction cache */
	or1k_jtag_write_cpu(&or1k->jtag,
			OR1K_ICBIR_CPU_REG_ADD, 1, &breakpoint->address);

	return ERROR_OK;
}

static int or1k_remove_breakpoint(struct target *target,
				  struct breakpoint *breakpoint)
{
	LOG_DEBUG("Removing breakpoint: addr %08x, len %d, type %d, set: %d, id: %d",
		  breakpoint->address, breakpoint->length, breakpoint->type,
		  breakpoint->set, breakpoint->unique_id);

	struct or1k_common *or1k = target_to_or1k(target);

	/* Only support SW breakpoints for now. */
	if (breakpoint->type == BKPT_HARD)
		LOG_ERROR("HW breakpoints not supported for now. Doing SW breakpoint.");

	/* Replace the removed instruction */
	or1k_jtag_write_memory32(&or1k->jtag, 
				breakpoint->address , 
				 1,
				 (uint32_t*)breakpoint->orig_instr);

	/* invalidate instruction cache */
	or1k_jtag_write_cpu(&or1k->jtag,
			OR1K_ICBIR_CPU_REG_ADD, 1, &breakpoint->address);

	return ERROR_OK;
}

static int or1k_add_watchpoint(struct target *target, 
			       struct watchpoint *watchpoint)
{
	LOG_ERROR("%s: implement me", __func__);

	return ERROR_OK;
}

static int or1k_remove_watchpoint(struct target *target,
				  struct watchpoint *watchpoint)
{
	LOG_ERROR("%s: implement me", __func__);

	return ERROR_OK;
}

static int or1k_bulk_read_memory(struct target *target, uint32_t address,
		uint32_t count, const uint8_t *buffer)
{
	struct or1k_common *or1k = target_to_or1k(target);
	
	/* Count is in 4-byte words */
	LOG_DEBUG("address 0x%x count %d", address, count);
	/*printf("bulk read memory: address 0x%x count %d\n", address, count);*/

	/* Break it up into 4 byte blocks */

	uint32_t block_count_left = count;
	uint32_t block_count_address = address;
	uint8_t *block_count_buffer = (uint8_t*) buffer;

	const unsigned int blocks_per_round = 1024; /* some resonable value */
	unsigned int blocks_this_round;

	while (block_count_left)
	{

		blocks_this_round = (block_count_left > blocks_per_round) ?  
			blocks_per_round : block_count_left;

		or1k_jtag_read_memory32(&or1k->jtag, 
					 block_count_address , 
					 blocks_this_round,
					 (uint32_t*)block_count_buffer);

		block_count_left -= blocks_this_round;
		block_count_address += 4*blocks_per_round;
		block_count_buffer += 4*blocks_per_round;


	}
	return ERROR_OK;
}

static int or1k_bulk_write_memory(struct target *target, uint32_t address,
		uint32_t count, const uint8_t *buffer)
{
	struct or1k_common *or1k = target_to_or1k(target);
	
	/* Count is in 4-byte words */
	LOG_DEBUG("address 0x%x count %d", address, count);
	/*printf("bulk write memory: address 0x%x count %d\n",address, count);*/

	/* Break it up into 4 byte blocks */

	uint32_t block_count_left = count;
	uint32_t block_count_address = address;
	uint8_t *block_count_buffer = (uint8_t*) buffer;

	const unsigned int blocks_per_round = 1024; /* some reasonable value */
	unsigned int blocks_this_round;

	while (block_count_left)
	{

		blocks_this_round = (block_count_left > blocks_per_round) ?  
			blocks_per_round : block_count_left;

		or1k_jtag_write_memory32(&or1k->jtag, 
					 block_count_address , 
					 blocks_this_round,
					 (uint32_t*)block_count_buffer);

		block_count_left -= blocks_this_round;
		block_count_address += 4*blocks_per_round;
		block_count_buffer += 4*blocks_per_round;


	}
	return ERROR_OK;
}

static int or1k_read_memory(struct target *target, uint32_t address,
		uint32_t size, uint32_t count, uint8_t *buffer)
{
	struct or1k_common *or1k = target_to_or1k(target);

	LOG_DEBUG("address: 0x%8.8" PRIx32 ", size: 0x%8.8" PRIx32 ", count: 0x%8.8" PRIx32 "", address, size, count);

	if (target->state != TARGET_HALTED)
	{
		LOG_WARNING("target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	/* sanitize arguments */
	if (((size != 4) && (size != 2) && (size != 1)) || (count == 0) || 
	    !(buffer))
		return ERROR_COMMAND_SYNTAX_ERROR;

	if (((size == 4) && (address & 0x3u)) || ((size == 2) && 
						  (address & 0x1u)))
		return ERROR_TARGET_UNALIGNED_ACCESS;
	
	
	if (size == 4 && count > 1)
		return or1k_bulk_read_memory(target, address, count, buffer);
	
	switch (size)
	{
	case 4:
		return or1k_jtag_read_memory32(&or1k->jtag, address, 
					       count, 
					       (uint32_t*)(void *)buffer);
		break;
	case 2:
		return or1k_jtag_read_memory16(&or1k->jtag, address, 
					       count, 
					       (uint16_t*)(void *)buffer);
		break;
	case 1:
		return or1k_jtag_read_memory8(&or1k->jtag, address, 
					      count, 
					      buffer);
		break;
	default:
		break;
	}

	return ERROR_OK;
}

static int or1k_write_memory(struct target *target, uint32_t address,
		uint32_t size, uint32_t count, const uint8_t *buffer)
{
	struct or1k_common *or1k = target_to_or1k(target);
	
	LOG_DEBUG("address: 0x%8.8" PRIx32 ", size: 0x%8.8" PRIx32 ", count: 0x%8.8" PRIx32 "", address, size, count);

	if (target->state != TARGET_HALTED)
	{
		LOG_WARNING("target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	/* sanitize arguments */
	if (((size != 4) && (size != 2) && (size != 1)) || (count == 0) || 
	    !(buffer))
		return ERROR_COMMAND_SYNTAX_ERROR;

	if (((size == 4) && (address & 0x3u)) || ((size == 2) && 
						  (address & 0x1u)))
		return ERROR_TARGET_UNALIGNED_ACCESS;

	if (size == 4 && count > 1)
		return or1k_bulk_write_memory(target, address, count, buffer);
	
	switch (size)
	{
	case 4:
		return or1k_jtag_write_memory32(&or1k->jtag, address, count, 
						(uint32_t*)(void *)buffer);
		break;
	case 2:
		return or1k_jtag_write_memory16(&or1k->jtag, address, count, 
						(uint16_t*)(void *)buffer);
		break;
	case 1:
		return or1k_jtag_write_memory8(&or1k->jtag, address, count, 
					       buffer);
		break;
	default:
		break;
	}

	return ERROR_OK;
}


static int or1k_init_target(struct command_context *cmd_ctx,
		struct target *target)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int i, way, reg_num;
	char name[32];

	or1k->jtag.tap = target->tap;

	memset(&or1k_core_reg_list_arch_info[104], 0, sizeof(struct or1k_core_reg) * 128 * 8);
	memset(&or1k_core_reg_list_arch_info[1139], 0, sizeof(struct or1k_core_reg) * 128 * 8);

	for (way = 0; way < 4; way++) {
		for (i = 0; i < 128; i++) {

			sprintf(name, "dtlbw%dmr%d", way, i);
			reg_num = 104 + i + (way * 256);
			or1k_core_reg_list_arch_info[reg_num].name = strdup(name);
			or1k_core_reg_list_arch_info[reg_num].list_num = reg_num;
			or1k_core_reg_list_arch_info[reg_num].spr_num = GROUP1 + 512 + i + (way * 256);
			or1k_core_reg_list_arch_info[reg_num].feature = "group1";

			sprintf(name, "dtlbw%dtr%d", way, i);
			reg_num = 232 + i + (way * 256);
			or1k_core_reg_list_arch_info[reg_num].name = strdup(name);
			or1k_core_reg_list_arch_info[reg_num].list_num = reg_num;
			or1k_core_reg_list_arch_info[reg_num].spr_num = GROUP1 + 640 + i + (way * 256);
			or1k_core_reg_list_arch_info[reg_num].feature = "group1";

			sprintf(name, "itlbw%dmr%d", way, i);
			reg_num = 1139 + i + (way * 256);
			or1k_core_reg_list_arch_info[reg_num].name = strdup(name);
			or1k_core_reg_list_arch_info[reg_num].list_num = reg_num;
			or1k_core_reg_list_arch_info[reg_num].spr_num = GROUP2 + 512 + i + (way * 256);
			or1k_core_reg_list_arch_info[reg_num].feature = "group2";

			sprintf(name, "itlbw%dtr%d", way, i);
			reg_num = 1267 + i + (way * 256);
			or1k_core_reg_list_arch_info[reg_num].name = strdup(name);
			or1k_core_reg_list_arch_info[reg_num].list_num = reg_num;
			or1k_core_reg_list_arch_info[reg_num].spr_num = GROUP2 + 640 + i + (way * 256);
			or1k_core_reg_list_arch_info[reg_num].feature = "group2";
		}
	}

	or1k_build_reg_cache(target);
	return ERROR_OK;
}

static int or1k_target_create(struct target *target, Jim_Interp *interp)
{
	struct or1k_common *or1k = calloc(1, sizeof(struct or1k_common));

	target->arch_info = or1k;

	return ERROR_OK;
}

static int or1k_examine(struct target *target)
{
	uint32_t cpu_cr;
	struct or1k_common *or1k = target_to_or1k(target);

	if (!target_was_examined(target))
	{
		target_set_examined(target);
		/* Do nothing special yet - Julius
		 */
		/*
		avr32_jtag_nexus_read(&ap7k->jtag, AVR32_OCDREG_DID, &devid);
		LOG_INFO("device id: %08x", devid);
		avr32_ocd_setbits(&ap7k->jtag, AVR32_OCDREG_DC,OCDREG_DC_DBE);
		avr32_jtag_nexus_read(&ap7k->jtag, AVR32_OCDREG_DS, &ds);
		*/
		/* check for processor halted */
	
		
		/* Possible specific to Mohor debug interface - others may 
		 * have to do something different here. 
		 */
 		or1k_jtag_read_cpu_cr(&or1k->jtag, &cpu_cr);
		if (cpu_cr & OR1K_MOHORDBGIF_CPU_CR_STALL) 
		{
			LOG_DEBUG("target is halted");
			target->state = TARGET_HALTED;
		}
		else
			target->state = TARGET_RUNNING;
	}

	return ERROR_OK;
}

int or1k_arch_state(struct target *target)
{
  /*
	struct or1k_common *or1k = target_to_or1k(target);
  */
	/*
	LOG_USER("target halted due to %s, pc: 0x%8.8" PRIx32 "",
                debug_reason_name(target), ap7k->jtag.dpc);
	*/
   	return ERROR_OK;
}

int or1k_get_gdb_reg_list(struct target *target, struct reg **reg_list[], 
			  int *reg_list_size, int list_type)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int i;

	LOG_DEBUG(" - ");

	if (list_type == G_REGISTERS_LIST) {
		/* We will have this called whenever GDB connects. */
		or1k_save_context(target);

		*reg_list_size = OR1KNUMCOREREGS;
		/* this is free()'d back in gdb_server.c's gdb_get_register_packet() */
		*reg_list = malloc(sizeof(struct reg*) * (*reg_list_size));

		for (i = 0; i < OR1KNUMCOREREGS; i++)
			(*reg_list)[i] = &or1k->core_cache->reg_list[i];
	} else {
		*reg_list_size = NBR_DEFINED_REGISTERS;
		*reg_list = malloc(sizeof(struct reg*) * (*reg_list_size));

		for (i = 0; i < NBR_DEFINED_REGISTERS; i++)
			(*reg_list)[i] = &or1k->core_cache->reg_list[i];
	}

	return ERROR_OK;

}

static int or1k_checksum_memory(struct target *target, uint32_t address,
		uint32_t count, uint32_t *checksum) {

	return ERROR_FAIL;

}

/* defined in server/gdb_server.h" */
extern struct connection *current_rsp_connection;
extern int gdb_rsp_resp_error;
#define ERROR_OK_NO_GDB_REPLY (-42)
#define OR1K_SPR_ACCESS_ALWAYS_AGAINST_HW 0

COMMAND_HANDLER(or1k_readspr_command_handler)
{
	struct target *target = get_current_target(CMD_CTX);
	struct or1k_common *or1k = target_to_or1k(target);
	uint32_t regnum, regval;
	int retval, i;
	int reg_cache_index = -1;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	
	//COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], regnum);

	if (1 != sscanf(CMD_ARGV[0], "%8x", &regnum))
		return ERROR_COMMAND_SYNTAX_ERROR;

	//LOG_DEBUG("adr 0x%08x",regnum);


	/* See if this register is in our cache and valid */
	
	struct or1k_core_reg *arch_info;
	for(i = 0; i < OR1KNUMCOREREGS; i++)
	{
		arch_info = (struct or1k_core_reg *)
			or1k->core_cache->reg_list[i].arch_info;
		if (arch_info->spr_num == regnum)
		{
			/* Reg is part of our cache. */
			reg_cache_index = i;
#if OR1K_SPR_ACCESS_ALWAYS_AGAINST_HW==0
			/* Is the local copy currently valid ? */
			if (or1k->core_cache->reg_list[i].valid == 1)
			{	
				regval = buf_get_u32(or1k->core_cache->reg_list[i].value,
						     0, 32);

				LOG_DEBUG("reading cached value");
			}
#else
			/* Now get the register value via JTAG */
			retval = or1k_jtag_read_cpu(&or1k->jtag, regnum, 1, &regval);
			if (retval != ERROR_OK)
				return retval;
		
#endif			
			break;
		}
	}

	/* Reg was not found in cache, or it was and the value wasn't valid. */
	if (reg_cache_index == -1 ||
	    (reg_cache_index != -1 && 
	     !(or1k->core_cache->reg_list[reg_cache_index].valid == 1)))
	{
		/* Now get the register value via JTAG */
		retval = or1k_jtag_read_cpu(&or1k->jtag, regnum, 1, &regval);
		
		if (retval != ERROR_OK)
			return retval;

	}
	
	
	if (current_rsp_connection != NULL)
	{
		char gdb_reply[9];
		sprintf(gdb_reply, "%8x", (unsigned int) regval);
		gdb_reply[8] = 0x0;
		
		//LOG_INFO("%s",gdb_reply);

		char *hex_buffer;
		int bin_size;
		
		bin_size = strlen(gdb_reply);
		
		hex_buffer = malloc(bin_size*2 + 1);
		if (hex_buffer == NULL)
			return ERROR_GDB_BUFFER_TOO_SMALL;
		
		for (i = 0; i < bin_size; i++)
			snprintf(hex_buffer + i*2, 3, "%2.2x", gdb_reply[i]);
		hex_buffer[bin_size*2] = 0;
		
		gdb_put_packet(current_rsp_connection, hex_buffer, 
			       bin_size*2);
		
		free(hex_buffer);
		
		gdb_rsp_resp_error = ERROR_OK_NO_GDB_REPLY;
	} 



	/* Reg part of local core cache, but not valid, so update it */
	if (reg_cache_index != -1 && 
	    !(or1k->core_cache->reg_list[reg_cache_index].valid==1))
	{
		/* Set the value in the core reg array */
		or1k->core_regs[reg_cache_index] = regval;
		/* Always update the register struct's value from the core 
		   array */
		or1k_read_core_reg(target, reg_cache_index);
	}


	return ERROR_OK;
}

COMMAND_HANDLER(or1k_writespr_command_handler)
{
	struct target *target = get_current_target(CMD_CTX);
	struct or1k_common *or1k = target_to_or1k(target);
	uint32_t regnum, regval;
	int retval;
	int i;

	if (CMD_ARGC != 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if (1 != sscanf(CMD_ARGV[0], "%8x", &regnum))
		return ERROR_COMMAND_SYNTAX_ERROR;
	
	if (1 != sscanf(CMD_ARGV[1], "%8x", &regval))
		return ERROR_COMMAND_SYNTAX_ERROR;

	LOG_DEBUG("adr 0x%08x val 0x%08x",regnum, regval);

	/* Determine if this SPR is part of our cache */
	struct or1k_core_reg *arch_info;
	for(i = 0; i < OR1KNUMCOREREGS; i++)
	{
		arch_info = (struct or1k_core_reg *)
			or1k->core_cache->reg_list[i].arch_info;
		if (arch_info->spr_num == regnum)
		{
			/* Reg is part of our cache. */
			or1k->core_cache->reg_list[i].valid = 1;
			or1k->core_cache->reg_list[i].dirty = 1;
			buf_set_u32(or1k->core_cache->reg_list[i].value, 0, 32,
				    regval);

			LOG_DEBUG("caching written value");
#if OR1K_SPR_ACCESS_ALWAYS_AGAINST_HW==0
			return ERROR_OK;
#else
			/* Break so we go on to actually do the write */
			break;
#endif
		}
	}

	
#if 0
	uint32_t verify_regval;

	while(1){

	/* Now set the register via JTAG */
	retval = or1k_jtag_write_cpu(&or1k->jtag, regnum, 1, &regval);

	if (retval != ERROR_OK)
		return retval;

	/* Now read back the register via JTAG */
	retval = or1k_jtag_read_cpu(&or1k->jtag, regnum, 1, &verify_regval);

	if (retval != ERROR_OK)
		return retval;

	LOG_DEBUG("written: %08x read: %08x",regval_be, verify_regval);
	
	if (regval == verify_regval)
		break;
	}

#else

	/* Now set the register via JTAG */
	retval = or1k_jtag_write_cpu(&or1k->jtag, regnum, 1, &regval);
	
	if (retval != ERROR_OK)
		return retval;
#endif	

	return ERROR_OK;
}




static const struct command_registration or1k_spr_command_handlers[] = {
	{
		"readspr",
		.handler = or1k_readspr_command_handler,
		.mode = COMMAND_ANY,
		.usage = "sprnum",
		.help = "read OR1k special purpose register sprnum",
	},
	{
		"writespr",
		.handler = or1k_writespr_command_handler,
		.mode = COMMAND_ANY,
		.usage = "sprnum value",
		.help = "write value to OR1k special purpose register sprnum",
	},
	COMMAND_REGISTRATION_DONE
};

const struct command_registration or1k_command_handlers[] = {
	{
		.chain = or1k_spr_command_handlers,
	},
	COMMAND_REGISTRATION_DONE
};


struct target_type or1k_target =
{
	.name = "or1k",

	.poll = or1k_poll,
	.arch_state = or1k_arch_state,

	.target_request_data = NULL,

	.halt = or1k_halt,
	.resume = or1k_resume,
	.step = or1k_step,

	.assert_reset = or1k_assert_reset,
	.deassert_reset = or1k_deassert_reset,
	.soft_reset_halt = or1k_soft_reset_halt,

	.get_gdb_reg_list = or1k_get_gdb_reg_list,

	.read_memory = or1k_read_memory,
	.write_memory = or1k_write_memory,
	.bulk_write_memory = or1k_bulk_write_memory,
	.checksum_memory = or1k_checksum_memory,

	.generate_tdesc_file = or1k_generate_tdesc,

	.commands = or1k_command_handlers,
	.add_breakpoint = or1k_add_breakpoint,
	.remove_breakpoint = or1k_remove_breakpoint,
	.add_watchpoint = or1k_add_watchpoint,
	.remove_watchpoint = or1k_remove_watchpoint,

	.target_create = or1k_target_create,
	.init_target = or1k_init_target,
	.examine = or1k_examine,
};
