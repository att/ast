# regression tests for the codex(1) sum methods

TITLE + sum

KEEP "*.dat"

function DATA
{
	typeset f
	integer i
	typeset -i8 n
	for f
	do	test -f $f && continue
		case $f in
		big.dat)for ((i = 0; i <= 10000; i++))
			do	print $i
			done
			;;
		chars.dat)
			typeset -i8 o
			for ((o = 0; o < 256; o++))
			do	print -f "\\${o#8#}"
			done
			;;
		xyz.dat)print x
			print y
			print z
			;;
		zero.dat)
			for ((n = 0; n < 256; n++))
			do	print -f "\\0"
			done
			;;
		zyx.dat)print z
			print y
			print x
			;;
		esac > $f
	done
}

TEST 01 'old att'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-att
		INPUT -
		IGNORE OUTPUT
		ERROR - 00000000
	EXEC	-e sum-att
		SAME INPUT xyz.dat
		ERROR - 00000189
	EXEC	-e sum-att
		SAME INPUT zyx.dat
		ERROR - 00000189
	EXEC	-e sum-att
		SAME INPUT big.dat
		ERROR - 0000c2bb
	EXEC	-e sum-att
		SAME INPUT chars.dat
		ERROR - 00007f80
	EXEC	-e sum-att
		SAME INPUT zero.dat
		ERROR - 00000000

TEST 02 'old bsd'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-bsd
		INPUT -
		IGNORE OUTPUT
		ERROR - 00000000
	EXEC	-e sum-bsd
		SAME INPUT xyz.dat
		ERROR - 0000005d
	EXEC	-e sum-bsd
		SAME INPUT zyx.dat
		ERROR - 0000105c
	EXEC	-e sum-bsd
		SAME INPUT big.dat
		ERROR - 0000c5d7
	EXEC	-e sum-bsd
		SAME INPUT chars.dat
		ERROR - 00000200
	EXEC	-e sum-bsd
		SAME INPUT zero.dat
		ERROR - 00000000

TEST 03 'ast memsum'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-ast
		INPUT -
		IGNORE OUTPUT
		ERROR - 00000000
	EXEC	-e sum-ast
		SAME INPUT xyz.dat
		ERROR - 13e35657
	EXEC	-e sum-ast
		SAME INPUT zyx.dat
		ERROR - 32559217
	EXEC	-e sum-ast
		SAME INPUT big.dat
		ERROR - af81083b
	EXEC	-e sum-ast
		SAME INPUT chars.dat
		ERROR - 9d02f880
	EXEC	-e sum-ast
		SAME INPUT zero.dat
		ERROR - b119c100

TEST 04 'zip crc'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-zip
		INPUT -
		IGNORE OUTPUT
		ERROR - 00000000
	EXEC	-e sum-zip
		SAME INPUT xyz.dat
		ERROR - 32a6240c
	EXEC	-e sum-zip
		SAME INPUT zyx.dat
		ERROR - 4d58e785
	EXEC	-e sum-zip
		SAME INPUT big.dat
		ERROR - d9b25527
	EXEC	-e sum-zip
		SAME INPUT chars.dat
		ERROR - 29058c73
	EXEC	-e sum-zip
		SAME INPUT zero.dat
		ERROR - 0d968558

TEST 05 'posix cksum'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-cksum
		INPUT -
		IGNORE OUTPUT
		ERROR - ffffffff
	EXEC	-e sum-cksum
		SAME INPUT xyz.dat
		ERROR - 7f9d010f
	EXEC	-e sum-cksum
		SAME INPUT zyx.dat
		ERROR - e2828823
	EXEC	-e sum-cksum
		SAME INPUT big.dat
		ERROR - 1f7fa105
	EXEC	-e sum-cksum
		SAME INPUT chars.dat
		ERROR - 4e4dc3a1
	EXEC	-e sum-cksum
		SAME INPUT zero.dat
		ERROR - fb3ee248

TEST 06 'md5 message digest'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-md5
		INPUT -
		IGNORE OUTPUT
		ERROR - d41d8cd98f00b204e9800998ecf8427e
	EXEC	-e sum-md5
		SAME INPUT xyz.dat
		ERROR - 5c37d4d5cc8d74de8ed81fc394a56c0e
	EXEC	-e sum-md5
		SAME INPUT zyx.dat
		ERROR - 30c4d234a30ae1665d3e63cbfac9ade9
	EXEC	-e sum-md5
		SAME INPUT big.dat
		ERROR - 4633277f9842941660fbd0a681b1e656
	EXEC	-e sum-md5
		SAME INPUT chars.dat
		ERROR - e2c865db4162bed963bfaa9ef6ac18f0
	EXEC	-e sum-md5
		SAME INPUT zero.dat
		ERROR - 348a9791dc41b89796ec3808b5b5262f
	EXEC	-e sum-md5
		INPUT -n - abc
		ERROR - 900150983cd24fb0d6963f7d28e17f72
	EXEC	-e sum-md5
		INPUT -n - abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq
		ERROR - 8215ef0796a20bcaaae116d3876c664a

TEST 07 'SHA-1 secure hash algorithm 1'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-sha1
		INPUT -
		IGNORE OUTPUT
		ERROR - da39a3ee5e6b4b0d3255bfef95601890afd80709
	EXEC	-e sum-sha1
		SAME INPUT xyz.dat
		ERROR - 83305e292107a8d1955ac0c0047912ff62c5d6dc
	EXEC	-e sum-sha1
		SAME INPUT zyx.dat
		ERROR - f1bac0f6f8e8d09b07cbc04c2e70b1b606fb9dd5
	EXEC	-e sum-sha1
		SAME INPUT big.dat
		ERROR - d3e7a9584187f017342dd759bc8f3061b74c5faf
	EXEC	-e sum-sha1
		SAME INPUT chars.dat
		ERROR - 4916d6bdb7f78e6803698cab32d1586ea457dfc8
	EXEC	-e sum-sha1
		SAME INPUT zero.dat
		ERROR - b376885ac8452b6cbf9ced81b1080bfd570d9b91
	EXEC	-e sum-sha1
		INPUT -n - abc
		ERROR - a9993e364706816aba3e25717850c26c9cd0d89d
	EXEC	-e sum-sha1
		INPUT -n - abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq
		ERROR - 84983e441c3bd26ebaae4aa1f95129e5e54670f1

TEST 08 'SHA-256 secure hash algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-sha-256
		INPUT -
		IGNORE OUTPUT
		ERROR - e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
	EXEC	-e sum-sha-256
		SAME INPUT xyz.dat
		ERROR - 81884b5f2cb68edc6286363dcc4699a913a2d5ba05818d0fdc43ba68bb990bd8
	EXEC	-e sum-sha-256
		SAME INPUT zyx.dat
		ERROR - 9c49771c064bd4fd1e0118f38fcf88c519c38370bcddcf7444fe6593f74005de
	EXEC	-e sum-sha-256
		SAME INPUT big.dat
		ERROR - 6cdce0d273f964e529fc3f90db732abb5c70b63a3a43151a2498bb41a23f2dff
	EXEC	-e sum-sha-256
		SAME INPUT chars.dat
		ERROR - 40aff2e9d2d8922e47afd4648e6967497158785fbd1da870e7110266bf944880
	EXEC	-e sum-sha-256
		SAME INPUT zero.dat
		ERROR - 5341e6b2646979a70e57653007a1f310169421ec9bdd9f1a5648f75ade005af1
	EXEC	-e sum-sha-256
		INPUT -n - abc
		ERROR - ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
	EXEC	-e sum-sha-256
		INPUT -n - abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq
		ERROR - 248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1

TEST 09 'SHA-384 secure hash algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-sha-384
		INPUT -
		IGNORE OUTPUT
		ERROR - 38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b
	EXEC	-e sum-sha-384
		SAME INPUT xyz.dat
		ERROR - 69ee1ce2ab4336efe6ef44c42b88d54978b0d8e15f5745a3e4c2a636c51b2e7cd27f14c9a99a56a695c63f05b01f8807
	EXEC	-e sum-sha-384
		SAME INPUT zyx.dat
		ERROR - 2a2a803235971c4bd86019e2370c77a16a7461ab44cef3fa374bc5720836ba46834192dc1e9557f926dee3d70f9f39fb
	EXEC	-e sum-sha-384
		SAME INPUT big.dat
		ERROR - 0a5aa14305020b93c982af350fba28dfbbe9ffe1e8691a179de30d899a24a9314fbe50eeb527ec379aaadc94c8f5bff6
	EXEC	-e sum-sha-384
		SAME INPUT chars.dat
		ERROR - ffdaebff65ed05cf400f0221c4ccfb4b2104fb6a51f87e40be6c4309386bfdec2892e9179b34632331a59592737db5c5
	EXEC	-e sum-sha-384
		SAME INPUT zero.dat
		ERROR - 983980373213482dd5c9a5a424db89418e3344c459fa31a356e42eaa28544ca01b9839f6593c9e5d79fd439b5da6ebef
	EXEC	-e sum-sha-384
		INPUT -n - abc
		ERROR - cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7
	EXEC	-e sum-sha-384
		INPUT -n - abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq
		ERROR - 3391fdddfc8dc7393707a65b1b4709397cf8b1d162af05abfe8f450de5f36bc6b0455a8520bc4e6f5fe95b1fe3c8452b

TEST 10 'SHA-512 secure hash algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-e sum-sha-512
		INPUT -
		IGNORE OUTPUT
		ERROR - cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e
	EXEC	-e sum-sha-512
		SAME INPUT xyz.dat
		ERROR - bd4f98333e7adec90b6e3e292f7a6386a509b443dbb374fb705b9354552b4d438fc31023b5b853f0a88e670c89392b05044f4b9b4dea3593fc871c82afe65891
	EXEC	-e sum-sha-512
		SAME INPUT zyx.dat
		ERROR - 25bd08f694f4ac12441f2e526e017556ca6f22ddf38cbe0519f24381697ebcaa9a44002fe759677d7271a24342359329a9a626444d7cfed9128e76bfbea95782
	EXEC	-e sum-sha-512
		SAME INPUT big.dat
		ERROR - 8dad61d786372cb82cdc1440a8e52062f2ce322d865eb6aca388a90c327e16297b5c041d8316cee2b36123c4b6c7eb0a7d8c483ddc92fcb1c7bfc5093630198f
	EXEC	-e sum-sha-512
		SAME INPUT chars.dat
		ERROR - 1e7b80bc8edc552c8feeb2780e111477e5bc70465fac1a77b29b35980c3f0ce4a036a6c9462036824bd56801e62af7e9feba5c22ed8a5af877bf7de117dcac6d
	EXEC	-e sum-sha-512
		SAME INPUT zero.dat
		ERROR - 693f95d58383a6162d2aab49eb60395dcc4bb22295120caf3f21e3039003230b287c566a03c7a0ca5accaed2133c700b1cb3f82edf8adcbddc92b4f9fb9910c6
	EXEC	-e sum-sha-512
		INPUT -n - abc
		ERROR - ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f
	EXEC	-e sum-sha-512
		INPUT -n - abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq
		ERROR - 204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445
