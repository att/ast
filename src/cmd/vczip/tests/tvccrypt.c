/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"vctest.h"
#include	"vccrypto.h"

/* Data from Appendix B of FIPS-193.pdf */
static Vcchar_t	Plaindt[] = /* plaintext input */
{	0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
	0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
};
static Vcchar_t	Plain2dt[] = /* twice above plaintext */
{	0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
	0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34,
	0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
	0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
};
#if 0
static Vcchar_t Cryptdt[] = /* encrypted data */
{	0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
	0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
};
#endif
static Vcchar_t	Cipherkey[] = /* cipher key */
{	0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
	0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
#if 0
static Vcchar_t	Expandedkey[] = /* expanded key */
{	0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
	0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,

	0xa0, 0xfa, 0xfe, 0x17, 0x88, 0x54, 0x2c, 0xb1,
	0x23, 0xa3, 0x39, 0x39, 0x2a, 0x6c, 0x76, 0x05,

	0xf2, 0xc2, 0x95, 0xf2, 0x7a, 0x96, 0xb9, 0x43,
	0x59, 0x35, 0x80, 0x7a, 0x73, 0x59, 0xf6, 0x7f,

	0x3d, 0x80, 0x47, 0x7d, 0x47, 0x16, 0xfe, 0x3e,
	0x1e, 0x23, 0x7e, 0x44, 0x6d, 0x7a, 0x88, 0x3b,

	0xef, 0x44, 0xa5, 0x41, 0xa8, 0x52, 0x5b, 0x7f,
	0xb6, 0x71, 0x25, 0x3b, 0xdb, 0x0b, 0xad, 0x00,

	0xd4, 0xd1, 0xc6, 0xf8, 0x7c, 0x83, 0x9d, 0x87,
	0xca, 0xf2, 0xb8, 0xbc, 0x11, 0xf9, 0x15, 0xbc,

	0x6d, 0x88, 0xa3, 0x7a, 0x11, 0x0b, 0x3e, 0xfd,
	0xdb, 0xf9, 0x86, 0x41, 0xca, 0x00, 0x93, 0xfd,

	0x4e, 0x54, 0xf7, 0x0e, 0x5f, 0x5f, 0xc9, 0xf3,
	0x84, 0xa6, 0x4f, 0xb2, 0x4e, 0xa6, 0xdc, 0x4f,

	0xea, 0xd2, 0x73, 0x21, 0xb5, 0x8d, 0xba, 0xd2,
	0x31, 0x2b, 0xf5, 0x60, 0x7f, 0x8d, 0x29, 0x2f,

	0xac, 0x77, 0x66, 0xf3, 0x19, 0xfa, 0xdc, 0x21,
	0x28, 0xd1, 0x29, 0x41, 0x57, 0x5c, 0x00, 0x6e,

	0xd0, 0x14, 0xf9, 0xa8, 0xc9, 0xee, 0x25, 0x89,
	0xe1, 0x3f, 0x0c, 0xc8, 0xb6, 0x63, 0x0c, 0xa6
};
#endif

MAIN()
{
	Vcchar_t	*plaindt, *edt, *ddt;
	ssize_t		plainsz, esz, dsz, k;
	Vcchar_t	data[1024];
	Vcx_t		xx;
	Vcodex_t	*evc, *dvc;
	Vcdisc_t	vcdc;

	/* test the subpackage for encryption */
	if(vcxinit(&xx, Vcxaes128, Cipherkey, -(ssize_t)sizeof(Cipherkey)) < 0)
		terror("Initializing encryption handle");
	if((esz = vcxencode(&xx, Plaindt, sizeof(Plaindt), &edt)) < 0)
		terror("Encrypting test data");

	/* save data in case buffer gets destroyed */
	memcpy(data, edt, esz); edt = data;

	if((dsz = vcxdecode(&xx, edt, esz, &ddt)) != sizeof(Plaindt))
		terror("Wrong decrypted size");
	if(memcmp(ddt, Plaindt, dsz) != 0)
		terror("Wrong decrypted data");
	vcxstop(&xx);

	/* test transform */
	vcdc.data = Cipherkey; vcdc.size = -(ssize_t)sizeof(Cipherkey);
	vcdc.eventf = 0;
	if(!(evc = vcopen(&vcdc, Vccrypt, "aes128", 0, VC_ENCODE)) )
		terror("Can't open encryption handle");
	if(!(dvc = vcopen(&vcdc, Vccrypt, "aes128", 0, VC_DECODE)) )
		terror("Can't open decryption handle");
	for(k = 0; k < 4; ++k)
	{	if((esz = vcapply(evc, Plain2dt, sizeof(Plain2dt), &edt)) <= 0)
			terror("Encryption error");
		if((dsz = vcapply(dvc, edt, esz, &ddt)) <= 0)
			terror("Decryption error");
		if(dsz != sizeof(Plain2dt))
			terror("Wrong decrypted size");
		if(memcmp(ddt, Plain2dt, sizeof(Plain2dt)) != 0)
			terror("Wrong decrypted data");
	}
	vcclose(evc);
	vcclose(dvc);

	/* test chained blocks */
	vcdc.data = Cipherkey; vcdc.size = (ssize_t)sizeof(Cipherkey);
	vcdc.eventf = 0;
	if(!(evc = vcopen(&vcdc, Vccrypt, "aes128.chain", 0, VC_ENCODE)) )
		terror("Can't open encryption handle");
	if(!(dvc = vcopen(&vcdc, Vccrypt, "aes128.chain", 0, VC_DECODE)) )
		terror("Can't open decryption handle");
	for(k = 0; k < 4; ++k)
	{	if((esz = vcapply(evc, Plain2dt, sizeof(Plain2dt), &edt)) <= 0)
			terror("Encryption error");
		if((dsz = vcapply(dvc, edt, esz, &ddt)) <= 0)
			terror("Decryption error");
		if(dsz != sizeof(Plain2dt))
			terror("Wrong decrypted size");
		if(memcmp(ddt, Plain2dt, sizeof(Plain2dt)) != 0)
			terror("Wrong decrypted data");
	}
	vcclose(evc);
	vcclose(dvc);

	/* test 128-bit transform with use of password */
	vcdc.data = "PhongBinhIan"; vcdc.size = 0;
	vcdc.eventf = 0;
	if(!(evc = vcopen(&vcdc, Vccrypt, "aes128", 0, VC_ENCODE)) )
		terror("Can't open encryption handle");
	if(!(dvc = vcopen(&vcdc, Vccrypt, "aes128", 0, VC_DECODE)) )
		terror("Can't open decryption handle");
	plaindt = "NguyenV, we love you!"; plainsz = strlen((char*)plaindt);
	for(k = 0; k < 4; ++k)
	{	if((esz = vcapply(evc, plaindt, plainsz, &edt)) <= 0)
			terror("Encryption error");
		if((dsz = vcapply(dvc, edt, esz, &ddt)) <= 0)
			terror("Decryption error");
		if(dsz != plainsz)
			terror("Wrong decrypted size");
		if(memcmp(ddt, plaindt, plainsz) != 0)
			terror("Wrong decrypted data");
	}
	vcclose(evc);
	vcclose(dvc);

	/* test 192-bit transform with use of password */
	vcdc.data = "PhongBinhIan"; vcdc.size = 0;
	vcdc.eventf = 0;
	if(!(evc = vcopen(&vcdc, Vccrypt, "aes192.chain", 0, VC_ENCODE)) )
		terror("Can't open encryption handle");
	if(!(dvc = vcopen(&vcdc, Vccrypt, "aes192.chain", 0, VC_DECODE)) )
		terror("Can't open decryption handle");
	plaindt = "NguyenV, we love you!"; plainsz = strlen((char*)plaindt);
	for(k = 0; k < 4; ++k)
	{	if((esz = vcapply(evc, plaindt, plainsz, &edt)) <= 0)
			terror("Encryption error");
		if((dsz = vcapply(dvc, edt, esz, &ddt)) <= 0)
			terror("Decryption error");
		if(dsz != plainsz)
			terror("Wrong decrypted size");
		if(memcmp(ddt, plaindt, plainsz) != 0)
			terror("Wrong decrypted data");
	}
	vcclose(evc);
	vcclose(dvc);

	/* test 256-bit transform with use of password */
	vcdc.data = "PhongBinhIan"; vcdc.size = 0;
	vcdc.eventf = 0;
	if(!(evc = vcopen(&vcdc, Vccrypt, "aes256.chain", 0, VC_ENCODE)) )
		terror("Can't open encryption handle");
	if(!(dvc = vcopen(&vcdc, Vccrypt, "aes256.chain", 0, VC_DECODE)) )
		terror("Can't open decryption handle");
	plaindt = "NguyenV, we love you!"; plainsz = strlen((char*)plaindt);
	for(k = 0; k < 4; ++k)
	{	if((esz = vcapply(evc, plaindt, plainsz, &edt)) <= 0)
			terror("Encryption error");
		if((dsz = vcapply(dvc, edt, esz, &ddt)) <= 0)
			terror("Decryption error");
		if(dsz != plainsz)
			terror("Wrong decrypted size");
		if(memcmp(ddt, plaindt, plainsz) != 0)
			terror("Wrong decrypted data");
	}
	vcclose(evc);
	vcclose(dvc);

	exit(0);
}
