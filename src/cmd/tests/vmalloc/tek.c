/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2011 AT&T Intellectual Property          *
 *                      and is licensed under the                       *
 *                 Eclipse Public License, Version 1.0                  *
 *                    by AT&T Intellectual Property                     *
 *                                                                      *
 *                A copy of the License is available at                 *
 *          http://www.eclipse.org/org/documents/epl-v10.html           *
 *         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
 *                                                                      *
 *              Information and Software Systems Research               *
 *                            AT&T Research                             *
 *                           Florham Park NJ                            *
 *                                                                      *
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include "vmtest.h"

/* This test was from E Koutsofios to fix a bug with extending
** a region under Vmlast. On occasions, the bottom block does
** not get initialized correctly resulting in a Memory Fault
** because the "seg" field of the block is not set.
*/

static char *vms[1000];

tmain() {
    Vmalloc_t *vm;
    vm = vmopen(Vmdcsbrk, Vmlast, 0);

    vms[0] = vmalloc(Vmheap, 176);
    vms[1] = vmalloc(Vmheap, 1024);
    vms[2] = vmalloc(Vmheap, 176);
    vms[3] = vmalloc(Vmheap, 1024);
    vms[4] = vmalloc(Vmheap, 176);
    vms[5] = vmalloc(Vmheap, 1024);
    vms[6] = vmalloc(Vmheap, 72);
    vms[7] = vmalloc(Vmheap, 40);
    vms[8] = vmalloc(Vmheap, 2048);
    vms[9] = vmalloc(Vmheap, 176);
    vms[10] = vmalloc(Vmheap, 64);
    vms[11] = vmalloc(Vmheap, 1024);
    vms[12] = vmalloc(Vmheap, 64);
    vms[13] = vmalloc(Vmheap, 72);
    vms[14] = vmalloc(Vmheap, 40);
    vms[15] = vmalloc(Vmheap, 31);
    vms[16] = vmalloc(Vmheap, 2048);
    vms[17] = vmalloc(Vmheap, 29);
    vms[18] = vmalloc(Vmheap, 36);
    vms[19] = vmalloc(Vmheap, 112);
    vms[20] = vmalloc(Vmheap, 65536);
    vms[21] = vmalloc(Vmheap, 29851672);
    vms[22] = vmalloc(vm, 29850757);
    vms[23] = vmalloc(vm, 72);
    vms[24] = vmalloc(vm, 12);
    vms[25] = vmalloc(vm, 72);
    vms[26] = vmalloc(vm, 16);
    vms[27] = vmalloc(vm, 4);
    vms[28] = vmalloc(vm, 72);
    vms[29] = vmalloc(vm, 16);
    vms[30] = vmalloc(vm, 8);
    vms[31] = vmalloc(vm, 72);
    vms[32] = vmalloc(vm, 16);
    vms[33] = vmalloc(vm, 4);
    vms[34] = vmalloc(vm, 72);
    vms[35] = vmalloc(vm, 6);
    vms[36] = vmalloc(vm, 72);
    vms[37] = vmalloc(vm, 16);
    vms[38] = vmalloc(vm, 34);
    vms[39] = vmalloc(vm, 72);
    vms[29] = vmresize(vm, vms[29], 48, VM_RSCOPY);
    vms[40] = vmalloc(vm, 8);
    vms[41] = vmalloc(vm, 72);
    vms[42] = vmalloc(vm, 16);
    vms[43] = vmalloc(vm, 36);
    vms[44] = vmalloc(vm, 72);
    vms[45] = vmalloc(vm, 4);
    vms[46] = vmalloc(vm, 72);
    vms[47] = vmalloc(vm, 16);
    vms[48] = vmalloc(vm, 111);
    vms[49] = vmalloc(vm, 72);
    vms[50] = vmalloc(vm, 4);
    vms[51] = vmalloc(vm, 72);
    vms[52] = vmalloc(vm, 16);
    vms[53] = vmalloc(vm, 17);
    vms[54] = vmalloc(vm, 72);
    vms[55] = vmalloc(vm, 6);
    vms[56] = vmalloc(vm, 72);
    vms[57] = vmalloc(vm, 16);
    vms[58] = vmalloc(vm, 5);
    vms[59] = vmalloc(vm, 72);
    vms[60] = vmalloc(vm, 16);
    vms[61] = vmalloc(vm, 17);
    vms[62] = vmalloc(vm, 72);
    vms[63] = vmalloc(vm, 5);
    vms[64] = vmalloc(vm, 72);
    vms[65] = vmalloc(vm, 16);
    vms[66] = vmalloc(vm, 11);
    vms[67] = vmalloc(vm, 72);
    vms[57] = vmresize(vm, vms[57], 48, VM_RSCOPY);
    vms[68] = vmalloc(vm, 5);
    vms[69] = vmalloc(vm, 72);
    vms[70] = vmalloc(vm, 16);
    vms[71] = vmalloc(vm, 13);
    vms[72] = vmalloc(vm, 72);
    vms[29] = vmresize(vm, vms[29], 112, VM_RSCOPY);
    vms[73] = vmalloc(vm, 7);
    vms[74] = vmalloc(vm, 72);
    vms[75] = vmalloc(vm, 16);
    vms[76] = vmalloc(vm, 5);
    vms[77] = vmalloc(vm, 72);
    vms[78] = vmalloc(vm, 16);
    vms[79] = vmalloc(vm, 17);
    vms[80] = vmalloc(vm, 72);
    vms[81] = vmalloc(vm, 5);
    vms[82] = vmalloc(vm, 72);
    vms[83] = vmalloc(vm, 16);
    vms[84] = vmalloc(vm, 9);
    vms[85] = vmalloc(vm, 72);
    vms[75] = vmresize(vm, vms[75], 48, VM_RSCOPY);
    vms[86] = vmalloc(vm, 6);
    vms[87] = vmalloc(vm, 72);
    vms[88] = vmalloc(vm, 16);
    vms[89] = vmalloc(vm, 2);
    vms[90] = vmalloc(vm, 72);
    vms[91] = vmalloc(vm, 5);
    vms[92] = vmalloc(vm, 72);
    vms[93] = vmalloc(vm, 16);
    vms[94] = vmalloc(vm, 15);
    vms[95] = vmalloc(vm, 72);
    vms[96] = vmalloc(vm, 9);
    vms[97] = vmalloc(vm, 72);
    vms[98] = vmalloc(vm, 16);
    vms[99] = vmalloc(vm, 3);
    vms[100] = vmalloc(vm, 72);
    vms[101] = vmalloc(vm, 6);
    vms[102] = vmalloc(vm, 72);
    vms[103] = vmalloc(vm, 16);
    vms[104] = vmalloc(vm, 11);
    vms[105] = vmalloc(vm, 72);
    vms[98] = vmresize(vm, vms[98], 48, VM_RSCOPY);
    vms[106] = vmalloc(vm, 6);
    vms[107] = vmalloc(vm, 72);
    vms[108] = vmalloc(vm, 16);
    vms[109] = vmalloc(vm, 11);
    vms[110] = vmalloc(vm, 72);
    vms[111] = vmalloc(vm, 6);
    vms[112] = vmalloc(vm, 72);
    vms[113] = vmalloc(vm, 16);
    vms[114] = vmalloc(vm, 6);
    vms[115] = vmalloc(vm, 72);
    vms[116] = vmalloc(vm, 6);
    vms[117] = vmalloc(vm, 72);
    vms[118] = vmalloc(vm, 16);
    vms[119] = vmalloc(vm, 6);
    vms[120] = vmalloc(vm, 72);
    vms[121] = vmalloc(vm, 5);
    vms[122] = vmalloc(vm, 72);
    vms[123] = vmalloc(vm, 16);
    vms[124] = vmalloc(vm, 6);
    vms[125] = vmalloc(vm, 72);
    vms[98] = vmresize(vm, vms[98], 112, VM_RSCOPY);
    vms[126] = vmalloc(vm, 5);
    vms[127] = vmalloc(vm, 72);
    vms[128] = vmalloc(vm, 16);
    vms[129] = vmalloc(vm, 6);
    vms[130] = vmalloc(vm, 72);
    vms[131] = vmalloc(vm, 16);
    vms[132] = vmalloc(vm, 2);
    vms[133] = vmalloc(vm, 72);
    vms[134] = vmalloc(vm, 5);
    vms[135] = vmalloc(vm, 72);
    vms[136] = vmalloc(vm, 16);
    vms[137] = vmalloc(vm, 4);
    vms[138] = vmalloc(vm, 72);
    vms[139] = vmalloc(vm, 6);
    vms[140] = vmalloc(vm, 72);
    vms[141] = vmalloc(vm, 16);
    vms[142] = vmalloc(vm, 5);
    vms[143] = vmalloc(vm, 72);
    vms[144] = vmalloc(vm, 5);
    vms[145] = vmalloc(vm, 72);
    vms[146] = vmalloc(vm, 16);
    vms[147] = vmalloc(vm, 4);
    vms[148] = vmalloc(vm, 72);
    vms[149] = vmalloc(vm, 16);
    vms[150] = vmalloc(vm, 5);
    vms[151] = vmalloc(vm, 72);
    vms[152] = vmalloc(vm, 16);
    vms[153] = vmalloc(vm, 16);
    vms[154] = vmalloc(vm, 72);
    vms[155] = vmalloc(vm, 5);
    vms[156] = vmalloc(vm, 72);
    vms[157] = vmalloc(vm, 16);
    vms[158] = vmalloc(vm, 7);
    vms[159] = vmalloc(vm, 72);
    vms[149] = vmresize(vm, vms[149], 48, VM_RSCOPY);
    vms[160] = vmalloc(vm, 5);
    vms[161] = vmalloc(vm, 72);
    vms[162] = vmalloc(vm, 16);
    vms[163] = vmalloc(vm, 2);
    vms[164] = vmalloc(vm, 72);
    vms[165] = vmalloc(vm, 5);
    vms[166] = vmalloc(vm, 72);
    vms[167] = vmalloc(vm, 16);
    vms[168] = vmalloc(vm, 128);
    vms[169] = vmalloc(vm, 72);
    vms[170] = vmalloc(vm, 4);
    vms[171] = vmalloc(vm, 72);
    vms[172] = vmalloc(vm, 16);
    vms[173] = vmalloc(vm, 2);
    vms[174] = vmalloc(vm, 72);
    vms[175] = vmalloc(vm, 4);
    vms[176] = vmalloc(vm, 72);
    vms[177] = vmalloc(vm, 16);
    vms[178] = vmalloc(vm, 5);
    vms[179] = vmalloc(vm, 72);
    vms[180] = vmalloc(vm, 16);
    vms[181] = vmalloc(vm, 14);
    vms[182] = vmalloc(vm, 72);
    vms[183] = vmalloc(vm, 5);
    vms[184] = vmalloc(vm, 72);
    vms[185] = vmalloc(vm, 16);
    vms[186] = vmalloc(vm, 7);
    vms[187] = vmalloc(vm, 72);
    vms[177] = vmresize(vm, vms[177], 48, VM_RSCOPY);
    vms[188] = vmalloc(vm, 5);
    vms[189] = vmalloc(vm, 72);
    vms[190] = vmalloc(vm, 16);
    vms[191] = vmalloc(vm, 2);
    vms[192] = vmalloc(vm, 72);
    vms[193] = vmalloc(vm, 5);
    vms[194] = vmalloc(vm, 72);
    vms[195] = vmalloc(vm, 16);
    vms[196] = vmalloc(vm, 106);
    vms[197] = vmalloc(vm, 72);
    vms[198] = vmalloc(vm, 4);
    vms[199] = vmalloc(vm, 72);
    vms[200] = vmalloc(vm, 16);
    vms[201] = vmalloc(vm, 2);
    vms[202] = vmalloc(vm, 72);
    vms[146] = vmresize(vm, vms[146], 48, VM_RSCOPY);
    vms[203] = vmalloc(vm, 4);
    vms[204] = vmalloc(vm, 72);
    vms[205] = vmalloc(vm, 16);
    vms[206] = vmalloc(vm, 5);
    vms[207] = vmalloc(vm, 72);
    vms[208] = vmalloc(vm, 16);
    vms[209] = vmalloc(vm, 14);
    vms[210] = vmalloc(vm, 72);
    vms[211] = vmalloc(vm, 5);
    vms[212] = vmalloc(vm, 72);
    vms[213] = vmalloc(vm, 16);
    vms[214] = vmalloc(vm, 7);
    vms[215] = vmalloc(vm, 72);
    vms[205] = vmresize(vm, vms[205], 48, VM_RSCOPY);
    vms[216] = vmalloc(vm, 5);
    vms[217] = vmalloc(vm, 72);
    vms[218] = vmalloc(vm, 16);
    vms[219] = vmalloc(vm, 2);
    vms[220] = vmalloc(vm, 72);
    vms[221] = vmalloc(vm, 5);
    vms[222] = vmalloc(vm, 72);
    vms[223] = vmalloc(vm, 16);
    vms[224] = vmalloc(vm, 106);
    vms[225] = vmalloc(vm, 72);
    vms[226] = vmalloc(vm, 4);
    vms[227] = vmalloc(vm, 72);
    vms[228] = vmalloc(vm, 16);
    vms[229] = vmalloc(vm, 2);
    vms[230] = vmalloc(vm, 72);
    vms[231] = vmalloc(vm, 4);
    vms[232] = vmalloc(vm, 72);
    vms[233] = vmalloc(vm, 16);
    vms[234] = vmalloc(vm, 5);
    vms[235] = vmalloc(vm, 72);
    vms[236] = vmalloc(vm, 16);
    vms[237] = vmalloc(vm, 17);
    vms[238] = vmalloc(vm, 72);
    vms[239] = vmalloc(vm, 5);
    vms[240] = vmalloc(vm, 72);
    vms[241] = vmalloc(vm, 16);
    vms[242] = vmalloc(vm, 7);
    vms[243] = vmalloc(vm, 72);
    vms[233] = vmresize(vm, vms[233], 48, VM_RSCOPY);
    vms[244] = vmalloc(vm, 5);
    vms[245] = vmalloc(vm, 72);
    vms[246] = vmalloc(vm, 16);
    vms[247] = vmalloc(vm, 2);
    vms[248] = vmalloc(vm, 72);
    vms[249] = vmalloc(vm, 5);
    vms[250] = vmalloc(vm, 72);
    vms[251] = vmalloc(vm, 16);
    vms[252] = vmalloc(vm, 106);
    vms[253] = vmalloc(vm, 72);
    vms[254] = vmalloc(vm, 4);
    vms[255] = vmalloc(vm, 72);
    vms[256] = vmalloc(vm, 16);
    vms[257] = vmalloc(vm, 2);
    vms[258] = vmalloc(vm, 72);
    vms[259] = vmalloc(vm, 4);
    vms[260] = vmalloc(vm, 72);
    vms[261] = vmalloc(vm, 16);
    vms[262] = vmalloc(vm, 5);
    vms[263] = vmalloc(vm, 72);
    vms[264] = vmalloc(vm, 16);
    vms[265] = vmalloc(vm, 17);
    vms[266] = vmalloc(vm, 72);
    vms[267] = vmalloc(vm, 5);
    vms[268] = vmalloc(vm, 72);
    vms[269] = vmalloc(vm, 16);
    vms[270] = vmalloc(vm, 7);
    vms[271] = vmalloc(vm, 72);
    vms[261] = vmresize(vm, vms[261], 48, VM_RSCOPY);
    vms[272] = vmalloc(vm, 5);
    vms[273] = vmalloc(vm, 72);
    vms[274] = vmalloc(vm, 16);
    vms[275] = vmalloc(vm, 2);
    vms[276] = vmalloc(vm, 72);
    vms[277] = vmalloc(vm, 5);
    vms[278] = vmalloc(vm, 72);
    vms[279] = vmalloc(vm, 16);
    vms[280] = vmalloc(vm, 106);
    vms[281] = vmalloc(vm, 72);
    vms[282] = vmalloc(vm, 4);
    vms[283] = vmalloc(vm, 72);
    vms[284] = vmalloc(vm, 16);
    vms[285] = vmalloc(vm, 2);
    vms[286] = vmalloc(vm, 72);
    vms[287] = vmalloc(vm, 4);
    vms[288] = vmalloc(vm, 72);
    vms[289] = vmalloc(vm, 16);
    vms[290] = vmalloc(vm, 5);
    vms[291] = vmalloc(vm, 72);
    vms[292] = vmalloc(vm, 16);
    vms[293] = vmalloc(vm, 19);
    vms[294] = vmalloc(vm, 72);
    vms[295] = vmalloc(vm, 5);
    vms[296] = vmalloc(vm, 72);
    vms[297] = vmalloc(vm, 16);
    vms[298] = vmalloc(vm, 7);
    vms[299] = vmalloc(vm, 72);
    vms[289] = vmresize(vm, vms[289], 48, VM_RSCOPY);
    vms[300] = vmalloc(vm, 5);
    vms[301] = vmalloc(vm, 72);
    vms[302] = vmalloc(vm, 16);
    vms[303] = vmalloc(vm, 3);
    vms[304] = vmalloc(vm, 72);
    vms[305] = vmalloc(vm, 5);
    vms[306] = vmalloc(vm, 72);
    vms[307] = vmalloc(vm, 16);
    vms[308] = vmalloc(vm, 130);
    vms[309] = vmalloc(vm, 72);
    vms[310] = vmalloc(vm, 4);
    vms[311] = vmalloc(vm, 72);
    vms[312] = vmalloc(vm, 16);
    vms[313] = vmalloc(vm, 2);
    vms[314] = vmalloc(vm, 72);
    vms[146] = vmresize(vm, vms[146], 112, VM_RSCOPY);
    vms[315] = vmalloc(vm, 4);
    vms[316] = vmalloc(vm, 72);
    vms[317] = vmalloc(vm, 16);
    vms[318] = vmalloc(vm, 5);
    vms[319] = vmalloc(vm, 72);
    vms[320] = vmalloc(vm, 16);
    vms[321] = vmalloc(vm, 14);
    vms[322] = vmalloc(vm, 72);
    vms[323] = vmalloc(vm, 5);
    vms[324] = vmalloc(vm, 72);
    vms[325] = vmalloc(vm, 16);
    vms[326] = vmalloc(vm, 7);
    vms[327] = vmalloc(vm, 72);
    vms[317] = vmresize(vm, vms[317], 48, VM_RSCOPY);
    vms[328] = vmalloc(vm, 5);
    vms[329] = vmalloc(vm, 72);
    vms[330] = vmalloc(vm, 16);
    vms[331] = vmalloc(vm, 2);
    vms[332] = vmalloc(vm, 72);
    vms[333] = vmalloc(vm, 5);
    vms[334] = vmalloc(vm, 72);
    vms[335] = vmalloc(vm, 16);
    vms[336] = vmalloc(vm, 106);
    vms[337] = vmalloc(vm, 72);
    vms[338] = vmalloc(vm, 4);
    vms[339] = vmalloc(vm, 72);
    vms[340] = vmalloc(vm, 16);
    vms[341] = vmalloc(vm, 2);
    vms[342] = vmalloc(vm, 72);
    vms[343] = vmalloc(vm, 4);
    vms[344] = vmalloc(vm, 72);
    vms[345] = vmalloc(vm, 16);
    vms[346] = vmalloc(vm, 5);
    vms[347] = vmalloc(vm, 72);
    vms[348] = vmalloc(vm, 16);
    vms[349] = vmalloc(vm, 14);
    vms[350] = vmalloc(vm, 72);
    vms[351] = vmalloc(vm, 5);
    vms[352] = vmalloc(vm, 72);
    vms[353] = vmalloc(vm, 16);
    vms[354] = vmalloc(vm, 7);
    vms[355] = vmalloc(vm, 72);
    vms[345] = vmresize(vm, vms[345], 48, VM_RSCOPY);
    vms[356] = vmalloc(vm, 5);
    vms[357] = vmalloc(vm, 72);
    vms[358] = vmalloc(vm, 16);
    vms[359] = vmalloc(vm, 2);
    vms[360] = vmalloc(vm, 72);
    vms[361] = vmalloc(vm, 5);
    vms[362] = vmalloc(vm, 72);
    vms[363] = vmalloc(vm, 16);
    vms[364] = vmalloc(vm, 106);
    vms[365] = vmalloc(vm, 72);
    vms[366] = vmalloc(vm, 4);
    vms[367] = vmalloc(vm, 72);
    vms[368] = vmalloc(vm, 16);
    vms[369] = vmalloc(vm, 2);
    vms[370] = vmalloc(vm, 72);
    vms[371] = vmalloc(vm, 4);
    vms[372] = vmalloc(vm, 72);
    vms[373] = vmalloc(vm, 16);
    vms[374] = vmalloc(vm, 5);
    vms[375] = vmalloc(vm, 72);
    vms[376] = vmalloc(vm, 16);
    vms[377] = vmalloc(vm, 9);
    vms[378] = vmalloc(vm, 72);
    vms[379] = vmalloc(vm, 5);
    vms[380] = vmalloc(vm, 72);
    vms[381] = vmalloc(vm, 16);
    vms[382] = vmalloc(vm, 7);
    vms[383] = vmalloc(vm, 72);
    vms[373] = vmresize(vm, vms[373], 48, VM_RSCOPY);
    vms[384] = vmalloc(vm, 5);
    vms[385] = vmalloc(vm, 72);
    vms[386] = vmalloc(vm, 16);
    vms[387] = vmalloc(vm, 2);
    vms[388] = vmalloc(vm, 72);
    vms[389] = vmalloc(vm, 5);
    vms[390] = vmalloc(vm, 72);
    vms[391] = vmalloc(vm, 16);
    vms[392] = vmalloc(vm, 98);
    vms[393] = vmalloc(vm, 72);
    vms[394] = vmalloc(vm, 4);
    vms[395] = vmalloc(vm, 72);
    vms[396] = vmalloc(vm, 16);
    vms[397] = vmalloc(vm, 2);
    vms[398] = vmalloc(vm, 72);
    vms[399] = vmalloc(vm, 4);
    vms[400] = vmalloc(vm, 72);
    vms[401] = vmalloc(vm, 16);
    vms[402] = vmalloc(vm, 5);
    vms[403] = vmalloc(vm, 72);
    vms[404] = vmalloc(vm, 16);
    vms[405] = vmalloc(vm, 9);
    vms[406] = vmalloc(vm, 72);
    vms[407] = vmalloc(vm, 5);
    vms[408] = vmalloc(vm, 72);
    vms[409] = vmalloc(vm, 16);
    vms[410] = vmalloc(vm, 7);
    vms[411] = vmalloc(vm, 72);
    vms[401] = vmresize(vm, vms[401], 48, VM_RSCOPY);
    vms[412] = vmalloc(vm, 5);
    vms[413] = vmalloc(vm, 72);
    vms[414] = vmalloc(vm, 16);
    vms[415] = vmalloc(vm, 3);
    vms[416] = vmalloc(vm, 72);
    vms[417] = vmalloc(vm, 5);
    vms[418] = vmalloc(vm, 72);
    vms[419] = vmalloc(vm, 16);
    vms[420] = vmalloc(vm, 98);
    vms[421] = vmalloc(vm, 72);
    vms[422] = vmalloc(vm, 4);
    vms[423] = vmalloc(vm, 72);
    vms[424] = vmalloc(vm, 16);
    vms[425] = vmalloc(vm, 2);
    vms[426] = vmalloc(vm, 72);
    vms[427] = vmalloc(vm, 4);
    vms[428] = vmalloc(vm, 72);
    vms[429] = vmalloc(vm, 16);
    vms[430] = vmalloc(vm, 5);
    vms[431] = vmalloc(vm, 72);
    vms[432] = vmalloc(vm, 16);
    vms[433] = vmalloc(vm, 16);
    vms[434] = vmalloc(vm, 72);
    vms[435] = vmalloc(vm, 5);
    vms[436] = vmalloc(vm, 72);
    vms[437] = vmalloc(vm, 16);
    vms[438] = vmalloc(vm, 7);
    vms[439] = vmalloc(vm, 72);
    vms[429] = vmresize(vm, vms[429], 48, VM_RSCOPY);
    vms[440] = vmalloc(vm, 5);
    vms[441] = vmalloc(vm, 72);
    vms[442] = vmalloc(vm, 16);
    vms[443] = vmalloc(vm, 2);
    vms[444] = vmalloc(vm, 72);
    vms[445] = vmalloc(vm, 5);
    vms[446] = vmalloc(vm, 72);
    vms[447] = vmalloc(vm, 16);
    vms[448] = vmalloc(vm, 49);
    vms[449] = vmalloc(vm, 72);
    vms[450] = vmalloc(vm, 4);
    vms[451] = vmalloc(vm, 72);
    vms[452] = vmalloc(vm, 16);
    vms[453] = vmalloc(vm, 2);
    vms[454] = vmalloc(vm, 72);
    vms[455] = vmalloc(vm, 4);
    vms[456] = vmalloc(vm, 72);
    vms[457] = vmalloc(vm, 16);
    vms[458] = vmalloc(vm, 5);
    vms[459] = vmalloc(vm, 72);
    vms[460] = vmalloc(vm, 16);
    vms[461] = vmalloc(vm, 16);
    vms[462] = vmalloc(vm, 72);
    vms[463] = vmalloc(vm, 5);
    vms[464] = vmalloc(vm, 72);
    vms[465] = vmalloc(vm, 16);
    vms[466] = vmalloc(vm, 7);
    vms[467] = vmalloc(vm, 72);
    vms[457] = vmresize(vm, vms[457], 48, VM_RSCOPY);
    vms[468] = vmalloc(vm, 5);
    vms[469] = vmalloc(vm, 72);
    vms[470] = vmalloc(vm, 16);
    vms[471] = vmalloc(vm, 3);
    vms[472] = vmalloc(vm, 72);
    vms[473] = vmalloc(vm, 5);
    vms[474] = vmalloc(vm, 72);
    vms[475] = vmalloc(vm, 16);
    vms[476] = vmalloc(vm, 49);
    vms[477] = vmalloc(vm, 72);
    vms[478] = vmalloc(vm, 4);
    vms[479] = vmalloc(vm, 72);
    vms[480] = vmalloc(vm, 16);
    vms[481] = vmalloc(vm, 2);
    vms[482] = vmalloc(vm, 72);
    vms[483] = vmalloc(vm, 4);
    vms[484] = vmalloc(vm, 72);
    vms[485] = vmalloc(vm, 16);
    vms[486] = vmalloc(vm, 5);
    vms[487] = vmalloc(vm, 72);
    vms[488] = vmalloc(vm, 16);
    vms[489] = vmalloc(vm, 55);
    vms[490] = vmalloc(vm, 72);
    vms[491] = vmalloc(vm, 5);
    vms[492] = vmalloc(vm, 72);
    vms[493] = vmalloc(vm, 16);
    vms[494] = vmalloc(vm, 7);
    vms[495] = vmalloc(vm, 72);
    vms[485] = vmresize(vm, vms[485], 48, VM_RSCOPY);
    vms[496] = vmalloc(vm, 5);
    vms[497] = vmalloc(vm, 72);
    vms[498] = vmalloc(vm, 16);
    vms[499] = vmalloc(vm, 2);
    vms[500] = vmalloc(vm, 72);
    vms[501] = vmalloc(vm, 5);
    vms[502] = vmalloc(vm, 72);
    vms[503] = vmalloc(vm, 16);
    vms[504] = vmalloc(vm, 135);
    vms[505] = vmalloc(vm, 72);
    vms[506] = vmalloc(vm, 4);
    vms[507] = vmalloc(vm, 72);
    vms[508] = vmalloc(vm, 16);
    vms[509] = vmalloc(vm, 2);
    vms[510] = vmalloc(vm, 72);
    vms[511] = vmalloc(vm, 4);
    vms[512] = vmalloc(vm, 72);
    vms[513] = vmalloc(vm, 16);
    vms[514] = vmalloc(vm, 5);
    vms[515] = vmalloc(vm, 72);
    vms[516] = vmalloc(vm, 16);
    vms[517] = vmalloc(vm, 55);
    vms[518] = vmalloc(vm, 72);
    vms[519] = vmalloc(vm, 5);
    vms[520] = vmalloc(vm, 72);
    vms[521] = vmalloc(vm, 16);
    vms[522] = vmalloc(vm, 7);
    vms[523] = vmalloc(vm, 72);
    vms[513] = vmresize(vm, vms[513], 48, VM_RSCOPY);
    vms[524] = vmalloc(vm, 5);
    vms[525] = vmalloc(vm, 72);
    vms[526] = vmalloc(vm, 16);
    vms[527] = vmalloc(vm, 2);
    vms[528] = vmalloc(vm, 72);
    vms[529] = vmalloc(vm, 5);
    vms[530] = vmalloc(vm, 72);
    vms[531] = vmalloc(vm, 16);
    vms[532] = vmalloc(vm, 135);
    vms[533] = vmalloc(vm, 72);
    vms[534] = vmalloc(vm, 4);
    vms[535] = vmalloc(vm, 72);
    vms[536] = vmalloc(vm, 16);
    vms[537] = vmalloc(vm, 2);
    vms[538] = vmalloc(vm, 72);
    vms[146] = vmresize(vm, vms[146], 240, VM_RSCOPY);
    vms[539] = vmalloc(vm, 4);
    vms[540] = vmalloc(vm, 72);
    vms[541] = vmalloc(vm, 16);
    vms[542] = vmalloc(vm, 5);
    vms[543] = vmalloc(vm, 72);
    vms[544] = vmalloc(vm, 16);
    vms[545] = vmalloc(vm, 55);
    vms[546] = vmalloc(vm, 72);
    vms[547] = vmalloc(vm, 5);
    vms[548] = vmalloc(vm, 72);
    vms[549] = vmalloc(vm, 16);
    vms[550] = vmalloc(vm, 7);
    vms[551] = vmalloc(vm, 72);
    vms[541] = vmresize(vm, vms[541], 48, VM_RSCOPY);
    vms[552] = vmalloc(vm, 5);
    vms[553] = vmalloc(vm, 72);
    vms[554] = vmalloc(vm, 16);
    vms[555] = vmalloc(vm, 2);
    vms[556] = vmalloc(vm, 72);
    vms[557] = vmalloc(vm, 5);
    vms[558] = vmalloc(vm, 72);

    texit(0);
}
