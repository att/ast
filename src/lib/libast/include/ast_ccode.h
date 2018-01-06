#ifndef _def_ccode_features
#define _def_ccode_features 1

#define CC_ASCII 1    /* ISO-8859-1            */
#define CC_EBCDIC_E 2 /* Xopen dd(1) EBCDIC        */
#define CC_EBCDIC_I 3 /* Xopen dd(1) IBM        */
#define CC_EBCDIC_O 4 /* IBM-1047 mvs OpenEdition    */
#define CC_EBCDIC_S 5 /* Siemens posix-bc        */
#define CC_EBCDIC_H 6 /* IBM-37 AS/400        */
#define CC_EBCDIC_M 7 /* IBM mvs cobol        */
#define CC_EBCDIC_U 8 /* microfocus cobol        */

#define CC_MAPS 8 /* number of code maps        */

#define CC_EBCDIC CC_EBCDIC_E
#define CC_EBCDIC1 CC_EBCDIC_E
#define CC_EBCDIC2 CC_EBCDIC_I
#define CC_EBCDIC3 CC_EBCDIC_O

#if '~' == 0137
#define CC_NATIVE CC_EBCDIC_E /* native character code    */

#elif '~' == 0176
#define CC_NATIVE CC_ASCII /* native character code    */

#elif '~' == 0241
#if '\n' == 0025
#define CC_NATIVE CC_EBCDIC_O /* native character code    */
#else
#if '[' == 0272
#define CC_NATIVE CC_EBCDIC_H /* native character code    */
#else
#define CC_NATIVE CC_EBCDIC_I /* native character code    */
#endif
#endif

#elif '~' == 0377
#define CC_NATIVE CC_EBCDIC_S /* native character code    */

#else
#if 'A' == 0301
#define CC_NATIVE CC_EBCDIC_O /* native character code    */
#else
#define CC_NATIVE CC_ASCII /* native character code    */
#endif
#endif

#if 'A' == 0101
#define CC_ALIEN CC_EBCDIC /* alien character code        */
#define CC_bel 0007        /* bel character        */
#define CC_esc 0033        /* esc character        */
#define CC_sub 0032        /* sub character        */
#define CC_vt 0013         /* vt character            */
#else
#define CC_ALIEN CC_ASCII /* alien character code        */
#define CC_bel 0057       /* bel character        */
#define CC_esc 0047       /* esc character        */
#define CC_sub 0077       /* sub character        */
#define CC_vt 0013        /* vt character            */
#endif

#endif
