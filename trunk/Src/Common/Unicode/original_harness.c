/*
 * Copyright 2001 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 *
 * harness.c
 *
 * This is a test harness for "ConvertUTF.c".  Compile this
 * and run without arguments.  It will exhaustively test
 * the conversion routines, and print a few lines of diagnostic
 * output.  You don't need to compile ConvertUTF.c itself, 
 * since it gets #included here along with the header.
 * Example of a compile line:
 *
 *		$	gcc -g harness.c -o harness
 *
 * Rev History: Rick McGowan, new file April 2001.
 *
 */

#define CVTUTF_DEBUG 1

#include <stdio.h>
#include "ConvertUTF.c"

/* ---------------------------------------------------------------------
	test01 - Spot check a few legal & illegal UTF-8 values only.
	This is not an exhaustive test, just a brief one that was
	used to develop the "isLegalUTF8" routine.

	Legal UTF-8 sequences are:

	1st----	2nd----	3rd----	4th----	Codepoints---

	00-7F				  0000-  007F
	C2-DF	80-BF			  0080-  07FF
	E0	A0-BF	80-BF		  0800-  0FFF
	E1-EF	80-BF	80-BF		  1000-  FFFF
	F0	90-BF	80-BF	80-BF	 10000- 3FFFF
	F1-F3	80-BF	80-BF	80-BF	 40000- FFFFF
	F4	80-8F	80-BF	80-BF	100000-10FFFF

   --------------------------------------------------------------------- */


struct utf8_test {
    Boolean utf8_legal;	/* is legal sequence? */
    int utf8_len;	/* length of sequence */
    unsigned char utf8_seq[5];	/* the sequence */
};

struct utf8_test utf8_testData[] = {
    { 1,	1,	{ 0x7A, 0x00, 0x00, 0x00, 0x00 }},	/* 0 */
    { 1,	2,	{ 0xC2, 0xAC, 0x00, 0x00, 0x00 }},	/* 1 */
    { 1,	2,	{ 0xDF, 0xB2, 0x00, 0x00, 0x00 }},	/* 2 */
    { 1,	3,	{ 0xE0, 0xA1, 0x81, 0x00, 0x00 }},	/* 3 */
    { 1,	3,	{ 0xE1, 0xAC, 0x90, 0x00, 0x00 }},	/* 4 */
    { 1,	3,	{ 0xF0, 0x93, 0xB2, 0xA1, 0x00 }},	/* 5 */
    { 1,	4,	{ 0xF1, 0x87, 0x9A, 0xB0, 0x00 }},	/* 6 */
    { 1,	4,	{ 0xF3, 0x88, 0x9B, 0xAD, 0x00 }},	/* 7 */
    { 1,	4,	{ 0xF4, 0x82, 0x89, 0x8F, 0x00 }},	/* 8 */

    { 0,	3,	{ 0x82, 0x00, 0x00, 0x00, 0x00 }},	/* 9 */
    { 0,	2,	{ 0xF8, 0xAC, 0x00, 0x00, 0x00 }},	/* 10 */
    { 0,	2,	{ 0xE1, 0xFC, 0xFF, 0x00, 0x00 }},	/* 11 */
    { 0,	3,	{ 0xC2, 0xFC, 0x00, 0x00, 0x00 }},	/* 12 */
    { 0,	3,	{ 0xE1, 0xC2, 0x81, 0x00, 0x00 }},	/* 13 */
    { 0,	2,	{ 0xC2, 0xC1, 0x00, 0x00, 0x00 }},	/* 14 */
    { 0,	2,	{ 0xC0, 0xAF, 0x00, 0x00, 0x00 }},	/* 15 */
    { 0,	3,	{ 0xE0, 0x9F, 0x80, 0x00, 0x00 }},	/* 16 */
    { 0,	4,	{ 0xF0, 0x93, 0xB2, 0xC1, 0x00 }},	/* 17 */
/* for all > 17 use "short" buffer lengths to detect over-run */
    { 0,	4,	{ 0xF0, 0x93, 0xB2, 0xC3, 0x00 }},	/* 18 use short buflen */
    { 0,	0,	{ 0x00, 0x00, 0x00, 0x00, 0x00 }}
};

int test01() {
	int i;
	int rval, wantVal1, wantVal2, gotVal1, gotVal2, len2;

	printf("Begin Test01\n"); fflush(stdout);

	rval = 0;
	for (i = 0; utf8_testData[i].utf8_len; i++) {
		wantVal1 = wantVal2 = utf8_testData[i].utf8_legal;
		gotVal1 = isLegalUTF8(&(utf8_testData[i].utf8_seq[0]), utf8_testData[i].utf8_len);
		/* use truncated length for tests over 17 */
		if (i <= 17) { len2 = 4; } else { len2 = utf8_testData[i].utf8_len-1; wantVal2 = 0; }
		gotVal2 = isLegalUTF8Sequence(&(utf8_testData[i].utf8_seq[0]), &(utf8_testData[i].utf8_seq[0])+len2);
		if ((gotVal1 != wantVal1) || (gotVal2 != wantVal2)) {
			printf("Test01 error: seq %d is %d & %d (should be %d & %d) for bytes (%x,%x,%x,%x,%x,) & len %d\n",
			i, gotVal1, gotVal2, wantVal1, wantVal2, utf8_testData[i].utf8_seq[0],
			utf8_testData[i].utf8_seq[1], utf8_testData[i].utf8_seq[2],
			utf8_testData[i].utf8_seq[3], utf8_testData[i].utf8_seq[4],
			utf8_testData[i].utf8_len);
			++rval;
		}
	}

	return (rval ? 0 : 1);
}


/* ---------------------------------------------------------------------
	test02 - Test round trip UTF32 -> UTF16 -> UTF8 -> UTF16 -> UTF32

	This is an exhaustive test of values 0 through 0x10FFFF.  It
	takes each integer value and converts from UTC4 through the
	other encoding forms, and back to UTR32, checking the results
	along the way.

	It does not check the un-paired low surrogates, except for
	the first low surrogate.  It intends to get that one illegal
	result, prints a message, and continues with tests.

   --------------------------------------------------------------------- */

int test02() {
	int i, n;
	ConversionResult result;
	UTF32 utf32_buf[2], utf32_result[2];
	UTF16 utf16_buf[3], utf16_result[3];
	UTF8 utf8_buf[8];
	UTF32 *utf32SourceStart, *utf32TargetStart;
	UTF16 *utf16SourceStart, *utf16TargetStart;
	UTF8 *utf8SourceStart, *utf8TargetStart;

	printf("Begin Test02\n"); fflush(stdout);

	for (i = 0; i <= 0x10FFFF; i++) {
		utf32_buf[0] = i; utf32_buf[1] = 0;
		utf32_result[0] = utf32_result[1] = 0;
		utf16_buf[0] = utf16_buf[1] = utf16_buf[2] = 0;
		utf16_result[0] = utf16_result[1] = utf16_result[2] = 0;
		for (n = 0; n < 8; n++) utf8_buf[n] = 0;

		utf32SourceStart = utf32_buf; utf32TargetStart = utf32_result;
		utf16TargetStart = utf16SourceStart = utf16_buf;
		utf8TargetStart = utf8SourceStart = utf8_buf;

		/*
		 * Test UTF32 -> UTF16
		 */
		result = ConvertUTF32toUTF16((const UTF32 **) &utf32SourceStart, &(utf32_buf[1]), &utf16TargetStart, &(utf16_buf[2]), strictConversion);
		if (i < UNI_SUR_HIGH_START || i > UNI_SUR_LOW_END) {
			/* skip result checking for all but 0000d800, which we know to be illegal */
			switch (result) {
			default: fprintf(stderr, "Test02A fatal error: result %d for input %08x\n", result, utf32_buf[0]); exit(1);
			case conversionOK: break;
			case sourceExhausted: printf("sourceExhausted\t"); break;
			case targetExhausted: printf("targetExhausted\t"); break;
			case sourceIllegal: printf("sourceIllegal\t"); break;
		    }
		}
		if (result != conversionOK) {
			if (i <= UNI_SUR_HIGH_START || i > UNI_SUR_LOW_END) {
				printf("Test02A for %d, input %08x, output %04x,%04x, result %d\n",
				    i, utf32_buf[0], utf16_buf[0], utf16_buf[1], result);
				if ((i != UNI_SUR_HIGH_START) || (result != sourceIllegal)) {
					return 0;
				} else {
					printf("!!! Test02A: note expected illegal result for 0x0000D800\n");
				}
			}
		}
		if (i > UNI_SUR_HIGH_START && i <= UNI_SUR_LOW_END) continue;

		/*
		 * Test UTF16 -> UTF8, with legality check on.  We check for everything except
		 * for unpaired low surrogates.  We do make one check that the lowest low
		 * surrogate, when unpaired, is illegal.
		 */
		result = ConvertUTF16toUTF8((const UTF16 **) &utf16SourceStart, &(utf16_buf[2]), &utf8TargetStart, &(utf8_buf[7]), strictConversion);
		switch (result) {
		default: fprintf(stderr, "Test02B fatal error: result %d for input %08x\n", result, utf32_buf[0]); exit(1);
		case conversionOK: break;
		case sourceExhausted: printf("sourceExhausted\t"); break;
		case targetExhausted: printf("targetExhausted\t"); break;
		case sourceIllegal: printf("sourceIllegal\t"); break;
		}
		if (result != conversionOK) {

			printf("Test02B for %d (0x%x), input %04x,%04x; output %s; result %d\n",
				i, utf32_buf[0], utf16_buf[0], utf16_buf[1], utf8_buf, result);
			if ((i != UNI_SUR_LOW_START) && (i != UNI_SUR_HIGH_START)) {
				return 0;
			} else {
				/* Note: This illegal result only happens if we remove the surrogate
				    check in Test02A.  So it shouldn't be seen unless that check and
				    the "continue" are removed in the test above.
				*/
				if (i == UNI_SUR_LOW_START)
				    printf("!!! Test02B: note expected illegal result for 0xDC00,0000\n");
				else if (i == UNI_SUR_HIGH_START)
				    printf("!!! Test02B: note expected illegal result for 0xD800,0000\n");
			}
		}
		if ((i == UNI_SUR_LOW_START) && result != sourceIllegal) {
			printf("Test02B for %d (0x%x), input %04x,%04x; output %s; result %d\n",
				i, utf32_buf[0], utf16_buf[0], utf16_buf[1], utf8_buf, result);
				printf("Test02B: expected illegal result for 0xDC00,0000 was not flagged illegal.\n");
				return 0;
		}

		if ((i >= UNI_SUR_HIGH_START) & (i <= UNI_SUR_LOW_END)) continue;

		/*
		 * Reset some result buffer pointers for the trip back.
		 */
		utf32SourceStart = utf32_buf; utf32TargetStart = utf32_result;
		utf16TargetStart = utf16SourceStart = utf16_result;
		utf8TargetStart = utf8SourceStart = utf8_buf;

		/*
		 * Test UTF8 -> UTF16, with legality check on.
		 */
		result = ConvertUTF8toUTF16((const UTF8 **) &utf8SourceStart, &(utf8_buf[trailingBytesForUTF8[utf8_buf[0]]+1]), &utf16TargetStart, &(utf16_buf[2]), strictConversion);
		switch (result) {
		default: fprintf(stderr, "Test02C fatal error: result %d for input %08x\n", result, utf32_buf[0]); exit(1);
		case conversionOK: break;
		case sourceExhausted: printf("sourceExhausted\t"); break;
		case targetExhausted: printf("targetExhausted\t"); break;
		case sourceIllegal: printf("sourceIllegal\t"); break;
		}
		if (result != conversionOK) {
			printf("Test02C for %d (0x%x), input %s; output %04x,%04x; result %d\n",
				i, utf32_buf[0], utf8_buf, utf16_buf[0], utf16_buf[1], result);
			return 0;
		}
		for (n = 0; n < 3; n++) { /* check that the utf16 result is the same as what went in. */
			if (utf16_buf[n] != utf16_result[n]) {
				printf("Test02C error: input = 0x%08x; utf16_buf = 0x%04x,0x%04x; utf16_result = 0x%04x,0x%04x\n",
					utf32_buf[0], utf16_buf[0], utf16_buf[1], utf16_result[0], utf16_result[1]);
				return 0;
			}
		}

		/*
		 * Test UTF16 -> UTF32, with legality check on.  If the result of our previous
		 * conversion gave us a "surrogate pair", then we need to convert 2 entities
		 * back to UTF32.
		 */
		if (utf16_result[0] >= UNI_SUR_HIGH_START && utf16_result[0] <= UNI_SUR_HIGH_END) {
			result = ConvertUTF16toUTF32((const UTF16 **) &utf16SourceStart, &(utf16_result[2]), &utf32TargetStart, &(utf32_result[1]), strictConversion);
		} else {
			result = ConvertUTF16toUTF32((const UTF16 **) &utf16SourceStart, &(utf16_result[1]), &utf32TargetStart, &(utf32_result[1]), strictConversion);
		}
		switch (result) {
		default: fprintf(stderr, "Test02D fatal error: result %d for input %08x\n", result, utf32_buf[0]); exit(1);
		case conversionOK: break;
		case sourceExhausted: printf("sourceExhausted\t"); break;
		case targetExhausted: printf("targetExhausted\t"); break;
		case sourceIllegal: printf("sourceIllegal\t"); break;
		}
		if (result != conversionOK) {
			printf("Test02D for %d (0x%x), input %04x,%04x; output %08x; result %d\n",
				i, utf32_buf[0], utf16_buf[0], utf16_buf[1], utf32_result[0], result);
			return 0;
		}

		/*
		 * Now, check the final round-trip value.
		 */
		if (utf32_buf[0] != utf32_result[0]) {
			printf("Test02E for %d: utf32 input %08x; trip output %08x (utf_16buf is %04x,%04x)\n", i, utf32_buf[0], utf32_result[0], utf16_buf[0], utf16_buf[1]);
			return 0;
		}


	}
	return 1;
}

/* ---------------------------------------------------------------------
	test03 - Test round trip UTF32 -> UTF8 -> UTF32

	This tests the functions that were not tested by test02 above.
	For each UTF32 value 0 through 0x10FFFF, it tests the conversion
	to UTF-8 and back.  The test is exhaustive.

   --------------------------------------------------------------------- */

int test03() {
	int i, n;
	ConversionResult result;
	UTF32 utf32_buf[2], utf32_result[2];
	UTF8 utf8_buf[8];
	UTF32 *utf32SourceStart, *utf32TargetStart;
	UTF8 *utf8SourceStart, *utf8TargetStart;

	printf("Begin Test03\n"); fflush(stdout);

	for (i = 0; i <= 0x10FFFF; i++) {
		/* Skip all surrogates except UNI_SUR_HIGH_START, which we test for illegality. */
		if (i > UNI_SUR_HIGH_START && i <= UNI_SUR_LOW_END) continue;

		utf32_buf[0] = i; utf32_buf[1] = 0;
		utf32_result[0] = utf32_result[1] = 0;
		for (n = 0; n < 8; n++) utf8_buf[n] = 0;

		utf32SourceStart = utf32_buf; utf32TargetStart = utf32_result;
		utf8TargetStart = utf8SourceStart = utf8_buf;

		/*
		 * Test UTF32 -> UTF8, with legality check on.
		 */
		result = ConvertUTF32toUTF8((const UTF32 **) &utf32SourceStart, &(utf32_buf[1]), & utf8TargetStart, &(utf8_buf[7]), strictConversion);
		switch (result) {
		default: fprintf(stderr, "Test03A fatal error: result %d for input %08x\n", result, utf32_buf[0]); exit(1);
		case conversionOK: break;
		case sourceExhausted: printf("sourceExhausted\t"); break;
		case targetExhausted: printf("targetExhausted\t"); break;
		case sourceIllegal: printf("sourceIllegal\t"); break;
		}
		if (result != conversionOK) {
			printf("Test03A for %d (0x%x); output %s; result %d\n",
				i, utf32_buf[0], utf8_buf, result);
			if (i != UNI_SUR_HIGH_START) {
				return 0;
			} else {
				printf("!!! Test03A: note expected illegal result for 0x0000D800\n");
			}
		}
		if ((i == UNI_SUR_HIGH_START) && result != sourceIllegal) {
			printf("Test03A for %d (0x%x); output %s; result %d\n",
				i, utf32_buf[0], utf8_buf, result);
				printf("Test03A: expected illegal result for 0x0000D800 was not flagged illegal.\n");
				return 0;
		}

		if ((i >= UNI_SUR_HIGH_START) & (i <= UNI_SUR_LOW_END)) continue;

		/*
		 * Reset some result buffer pointers for the trip back.
		 */
		utf32SourceStart = utf32_buf; utf32TargetStart = utf32_result;
		utf8TargetStart = utf8SourceStart = utf8_buf;

		/*
		 * Test UTF8 -> UTF32, with legality check on.
		 */
		result = ConvertUTF8toUTF32((const UTF8 **) &utf8SourceStart, &(utf8_buf[trailingBytesForUTF8[utf8_buf[0]]+1]), &utf32TargetStart, &(utf32_result[1]), strictConversion);
		switch (result) {
		default: fprintf(stderr, "Test03B fatal error: result %d for input %08x\n", result, utf32_buf[0]); exit(1);
		case conversionOK: break;
		case sourceExhausted: printf("sourceExhausted\t"); break;
		case targetExhausted: printf("targetExhausted\t"); break;
		case sourceIllegal: printf("sourceIllegal\t"); break;
		}
		if (result != conversionOK) {
			printf("Test03B for %d (0x%x), input %s; output 0x%08x; result %d\n",
				i, utf32_buf[0], utf8_buf, utf32_result[0], result);
			return 0;
		}

		/*
		 * Now, check the final round-trip value.
		 */
		if (utf32_buf[0] != utf32_result[0]) {
			printf("Test03C for %d: utf32 input %08x; utf8 buf %s; trip output %08x\n", i, utf32_buf[0], utf8_buf, utf32_result[0]);
			return 0;
		}
	}
	return 1;
}

/* --------------------------------------------------------------------- */

main() {
	printf("Three tests of round-trip conversions will be performed.\n");
	printf("Two illegal result messages are expected; one in test 02A; one in test 03A .\n\n");
	fflush(stdout);
	if (test01()) {	printf("******** Test01 succeeded without error. ********\n\n"); }
	else { printf("-------- Test01 failed. --------\n\n"); }
	if (test02()) { printf("******** Test02 succeeded without error. ********\n\n"); }
	else { printf("-------- Test02 failed. --------\n\n"); }
	if (test03()) { printf("******** Test03 succeeded without error. ********\n\n"); }
	else { printf("-------- Test03 failed. --------\n\n"); }
}
