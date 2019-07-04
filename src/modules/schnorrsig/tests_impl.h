/**********************************************************************
 * Copyright (c) 2018 Andrew Poelstra                                 *
 * Distributed under the MIT software license, see the accompanying   *
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/

#ifndef _SECP256K1_MODULE_SCHNORRSIG_TESTS_
#define _SECP256K1_MODULE_SCHNORRSIG_TESTS_

#include "secp256k1_schnorrsig.h"

void test_schnorrsig_serialize(void) {
    secp256k1_schnorrsig sig;
    unsigned char in[64];
    unsigned char out[64];

    memset(in, 0x12, 64);
    CHECK(secp256k1_schnorrsig_parse(ctx, &sig, in));
    CHECK(secp256k1_schnorrsig_serialize(ctx, out, &sig));
    CHECK(memcmp(in, out, 64) == 0);
}

void test_schnorrsig_api(secp256k1_scratch_space *scratch) {
    unsigned char sk1[32];
    unsigned char sk2[32];
    unsigned char sk3[32];
    unsigned char msg[32];
    unsigned char sig64[64];
    secp256k1_xonly_pubkey pk[3];
    secp256k1_xonly_pubkey zero_pk;
    secp256k1_schnorrsig sig;
    const secp256k1_schnorrsig *sigptr = &sig;
    const unsigned char *msgptr = msg;
    const secp256k1_xonly_pubkey *pkptr = &pk[0];
    const secp256k1_xonly_pubkey *zeroptr = &zero_pk;

    /** setup **/
    secp256k1_context *none = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    secp256k1_context *sign = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_context *vrfy = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);
    secp256k1_context *both = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    int ecount;

    secp256k1_context_set_error_callback(none, counting_illegal_callback_fn, &ecount);
    secp256k1_context_set_error_callback(sign, counting_illegal_callback_fn, &ecount);
    secp256k1_context_set_error_callback(vrfy, counting_illegal_callback_fn, &ecount);
    secp256k1_context_set_error_callback(both, counting_illegal_callback_fn, &ecount);
    secp256k1_context_set_illegal_callback(none, counting_illegal_callback_fn, &ecount);
    secp256k1_context_set_illegal_callback(sign, counting_illegal_callback_fn, &ecount);
    secp256k1_context_set_illegal_callback(vrfy, counting_illegal_callback_fn, &ecount);
    secp256k1_context_set_illegal_callback(both, counting_illegal_callback_fn, &ecount);

    secp256k1_rand256(sk1);
    secp256k1_rand256(sk2);
    secp256k1_rand256(sk3);
    secp256k1_rand256(msg);
    CHECK(secp256k1_xonly_pubkey_create(ctx, &pk[0], sk1) == 1);
    CHECK(secp256k1_xonly_pubkey_create(ctx, &pk[1], sk2) == 1);
    CHECK(secp256k1_xonly_pubkey_create(ctx, &pk[2], sk3) == 1);
    memset(&zero_pk, 0, sizeof(zero_pk));

    /** main test body **/
    ecount = 0;
    CHECK(secp256k1_schnorrsig_sign(none, &sig, msg, sk1, NULL, NULL) == 0);
    CHECK(ecount == 1);
    CHECK(secp256k1_schnorrsig_sign(vrfy, &sig, msg, sk1, NULL, NULL) == 0);
    CHECK(ecount == 2);
    CHECK(secp256k1_schnorrsig_sign(sign, &sig, msg, sk1, NULL, NULL) == 1);
    CHECK(ecount == 2);
    CHECK(secp256k1_schnorrsig_sign(sign, NULL, msg, sk1, NULL, NULL) == 0);
    CHECK(ecount == 3);
    CHECK(secp256k1_schnorrsig_sign(sign, &sig, NULL, sk1, NULL, NULL) == 0);
    CHECK(ecount == 4);
    CHECK(secp256k1_schnorrsig_sign(sign, &sig, msg, NULL, NULL, NULL) == 0);
    CHECK(ecount == 5);

    ecount = 0;
    CHECK(secp256k1_schnorrsig_serialize(none, sig64, &sig) == 1);
    CHECK(ecount == 0);
    CHECK(secp256k1_schnorrsig_serialize(none, NULL, &sig) == 0);
    CHECK(ecount == 1);
    CHECK(secp256k1_schnorrsig_serialize(none, sig64, NULL) == 0);
    CHECK(ecount == 2);
    CHECK(secp256k1_schnorrsig_parse(none, &sig, sig64) == 1);
    CHECK(ecount == 2);
    CHECK(secp256k1_schnorrsig_parse(none, NULL, sig64) == 0);
    CHECK(ecount == 3);
    CHECK(secp256k1_schnorrsig_parse(none, &sig, NULL) == 0);
    CHECK(ecount == 4);

    ecount = 0;
    CHECK(secp256k1_schnorrsig_verify(none, &sig, msg, &pk[0]) == 0);
    CHECK(ecount == 1);
    CHECK(secp256k1_schnorrsig_verify(sign, &sig, msg, &pk[0]) == 0);
    CHECK(ecount == 2);
    CHECK(secp256k1_schnorrsig_verify(vrfy, &sig, msg, &pk[0]) == 1);
    CHECK(ecount == 2);
    CHECK(secp256k1_schnorrsig_verify(vrfy, NULL, msg, &pk[0]) == 0);
    CHECK(ecount == 3);
    CHECK(secp256k1_schnorrsig_verify(vrfy, &sig, NULL, &pk[0]) == 0);
    CHECK(ecount == 4);
    CHECK(secp256k1_schnorrsig_verify(vrfy, &sig, msg, NULL) == 0);
    CHECK(ecount == 5);
    CHECK(secp256k1_schnorrsig_verify(vrfy, &sig, msg, &zero_pk) == 0);
    CHECK(ecount == 6);

    ecount = 0;
    CHECK(secp256k1_schnorrsig_verify_batch(none, scratch, &sigptr, &msgptr, &pkptr, 1) == 0);
    CHECK(ecount == 1);
    CHECK(secp256k1_schnorrsig_verify_batch(sign, scratch, &sigptr, &msgptr, &pkptr, 1) == 0);
    CHECK(ecount == 2);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, scratch, &sigptr, &msgptr, &pkptr, 1) == 1);
    CHECK(ecount == 2);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, NULL, &sigptr, &msgptr, &pkptr, 1) == 0);
    CHECK(ecount == 3);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, scratch, NULL, NULL, NULL, 0) == 1);
    CHECK(ecount == 3);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, scratch, NULL, &msgptr, &pkptr, 1) == 0);
    CHECK(ecount == 4);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, scratch, &sigptr, NULL, &pkptr, 1) == 0);
    CHECK(ecount == 5);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, scratch, &sigptr, &msgptr, NULL, 1) == 0);
    CHECK(ecount == 6);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, scratch, &sigptr, &msgptr, &pkptr, (size_t)1 << (sizeof(size_t)*8-1)) == 0);
    CHECK(ecount == 7);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, scratch, &sigptr, &msgptr, &pkptr, (uint32_t)1 << 31) == 0);
    CHECK(ecount == 8);
    CHECK(secp256k1_schnorrsig_verify_batch(vrfy, scratch, &sigptr, &msgptr, &zeroptr, 1) == 0);
    CHECK(ecount == 9);

    secp256k1_context_destroy(none);
    secp256k1_context_destroy(sign);
    secp256k1_context_destroy(vrfy);
    secp256k1_context_destroy(both);
}

/* Checks that hash initialized by secp256k1_musig_sha256_tagged has the
 * expected state. */
void test_schnorrsig_sha256_tagged(void) {
    char tag[10] = "BIPSchnorr";
    secp256k1_sha256 sha;
    secp256k1_sha256 sha_optimized;

    secp256k1_sha256_initialize_tagged(&sha, (unsigned char *) tag, sizeof(tag));
    secp256k1_schnorrsig_sha256_tagged(&sha_optimized);
    test_sha256_eq(&sha, &sha_optimized);
}

/* Helper function for schnorrsig_bip_vectors
 * Signs the message and checks that it's the same as expected_sig. */
void test_schnorrsig_bip_vectors_check_signing(const unsigned char *sk, const unsigned char *pk_serialized, const unsigned char *msg, const unsigned char *expected_sig) {
    secp256k1_schnorrsig sig;
    unsigned char serialized_sig[64];
    secp256k1_xonly_pubkey pk;

    CHECK(secp256k1_schnorrsig_sign(ctx, &sig, msg, sk, NULL, NULL));
    CHECK(secp256k1_schnorrsig_serialize(ctx, serialized_sig, &sig));
    CHECK(memcmp(serialized_sig, expected_sig, 64) == 0);

    CHECK(secp256k1_xonly_pubkey_parse(ctx, &pk, pk_serialized));
    CHECK(secp256k1_schnorrsig_verify(ctx, &sig, msg, &pk));
}

/* Helper function for schnorrsig_bip_vectors
 * Checks that both verify and verify_batch return the same value as expected. */
void test_schnorrsig_bip_vectors_check_verify(secp256k1_scratch_space *scratch, const unsigned char *pk_serialized, const unsigned char *msg32, const unsigned char *sig_serialized, int expected) {
    const unsigned char *msg_arr[1];
    const secp256k1_schnorrsig *sig_arr[1];
    const secp256k1_xonly_pubkey *pk_arr[1];
    secp256k1_xonly_pubkey pk;
    secp256k1_schnorrsig sig;

    CHECK(secp256k1_xonly_pubkey_parse(ctx, &pk, pk_serialized));
    CHECK(secp256k1_schnorrsig_parse(ctx, &sig, sig_serialized));

    sig_arr[0] = &sig;
    msg_arr[0] = msg32;
    pk_arr[0] = &pk;

    CHECK(expected == secp256k1_schnorrsig_verify(ctx, &sig, msg32, &pk));
    CHECK(expected == secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, 1));
}

/* Test vectors according to BIP-schnorr
 * (https://github.com/sipa/bips/blob/775cb2fd903cbfe459081fda49ada744ef3139be/bip-schnorr/test-vectors.csv).
 */
void test_schnorrsig_bip_vectors(secp256k1_scratch_space *scratch) {
    {
        /* Test vector 0 */
        const unsigned char sk[32] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
        };
        const unsigned char pk[32] = {
            0x79, 0xBE, 0x66, 0x7E, 0xF9, 0xDC, 0xBB, 0xAC,
            0x55, 0xA0, 0x62, 0x95, 0xCE, 0x87, 0x0B, 0x07,
            0x02, 0x9B, 0xFC, 0xDB, 0x2D, 0xCE, 0x28, 0xD9,
            0x59, 0xF2, 0x81, 0x5B, 0x16, 0xF8, 0x17, 0x98
        };
        const unsigned char msg[32] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        const unsigned char sig[64] = {
            0x52, 0x8F, 0x74, 0x57, 0x93, 0xE8, 0x47, 0x2C,
            0x03, 0x29, 0x74, 0x2A, 0x46, 0x3F, 0x59, 0xE5,
            0x8F, 0x3A, 0x3F, 0x1A, 0x4A, 0xC0, 0x9C, 0x28,
            0xF6, 0xF8, 0x51, 0x4D, 0x4D, 0x03, 0x22, 0xA2,
            0x58, 0xBD, 0x08, 0x39, 0x8F, 0x82, 0xCF, 0x67,
            0xB8, 0x12, 0xAB, 0x2C, 0x77, 0x17, 0xCE, 0x56,
            0x6F, 0x87, 0x7C, 0x2F, 0x87, 0x95, 0xC8, 0x46,
            0x14, 0x69, 0x78, 0xE8, 0xF0, 0x47, 0x82, 0xAE
        };
        test_schnorrsig_bip_vectors_check_signing(sk, pk, msg, sig);
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 1);
    }
    {
        /* Test vector 1 */
        const unsigned char sk[32] = {
            0xB7, 0xE1, 0x51, 0x62, 0x8A, 0xED, 0x2A, 0x6A,
            0xBF, 0x71, 0x58, 0x80, 0x9C, 0xF4, 0xF3, 0xC7,
            0x62, 0xE7, 0x16, 0x0F, 0x38, 0xB4, 0xDA, 0x56,
            0xA7, 0x84, 0xD9, 0x04, 0x51, 0x90, 0xCF, 0xEF
        };
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x13, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0x66, 0x7C, 0x2F, 0x77, 0x8E, 0x06, 0x16, 0xE6,
            0x11, 0xBD, 0x0C, 0x14, 0xB8, 0xA6, 0x00, 0xC5,
            0x88, 0x45, 0x51, 0x70, 0x1A, 0x94, 0x9E, 0xF0,
            0xEB, 0xFD, 0x72, 0xD4, 0x52, 0xD6, 0x4E, 0x84,
            0x41, 0x60, 0xBC, 0xFC, 0x3F, 0x46, 0x6E, 0xCB,
            0x8F, 0xAC, 0xD1, 0x9A, 0xDE, 0x57, 0xD8, 0x69,
            0x9D, 0x74, 0xE7, 0x20, 0x7D, 0x78, 0xC6, 0xAE,
            0xDC, 0x37, 0x99, 0xB5, 0x2A, 0x8E, 0x05, 0x98
        };
        test_schnorrsig_bip_vectors_check_signing(sk, pk, msg, sig);
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 1);
    }
    {
        /* Test vector 2 */
        const unsigned char sk[32] = {
            0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
            0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
            0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
            0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x14, 0xE5, 0xC9
        };
        const unsigned char pk[32] = {
            0xDD, 0x30, 0x8A, 0xFE, 0xC5, 0x77, 0x7E, 0x13,
            0x12, 0x1F, 0xA7, 0x2B, 0x9C, 0xC1, 0xB7, 0xCC,
            0x01, 0x39, 0x71, 0x53, 0x09, 0xB0, 0x86, 0xC9,
            0x60, 0xE1, 0x8F, 0xD9, 0x69, 0x77, 0x4E, 0xB8
        };
        const unsigned char msg[32] = {
            0x5E, 0x2D, 0x58, 0xD8, 0xB3, 0xBC, 0xDF, 0x1A,
            0xBA, 0xDE, 0xC7, 0x82, 0x90, 0x54, 0xF9, 0x0D,
            0xDA, 0x98, 0x05, 0xAA, 0xB5, 0x6C, 0x77, 0x33,
            0x30, 0x24, 0xB9, 0xD0, 0xA5, 0x08, 0xB7, 0x5C
        };
        const unsigned char sig[64] = {
            0x2D, 0x94, 0x1B, 0x38, 0xE3, 0x26, 0x24, 0xBF,
            0x0A, 0xC7, 0x66, 0x9C, 0x09, 0x71, 0xB9, 0x90,
            0x99, 0x4A, 0xF6, 0xF9, 0xB1, 0x84, 0x26, 0xBF,
            0x4F, 0x4E, 0x7E, 0xC1, 0x0E, 0x6C, 0xDF, 0x38,
            0x6C, 0xF6, 0x46, 0xC6, 0xDD, 0xAF, 0xCF, 0xA7,
            0xF1, 0x99, 0x3E, 0xEB, 0x2E, 0x4D, 0x66, 0x41,
            0x6A, 0xEA, 0xD1, 0xDD, 0xAE, 0x2F, 0x22, 0xD6,
            0x3C, 0xAD, 0x90, 0x14, 0x12, 0xD1, 0x16, 0xC6
        };
        test_schnorrsig_bip_vectors_check_signing(sk, pk, msg, sig);
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 1);
    }
    {
        /* Test vector 3 */
        const unsigned char sk[32] = {
            0x0B, 0x43, 0x2B, 0x26, 0x77, 0x93, 0x73, 0x81,
            0xAE, 0xF0, 0x5B, 0xB0, 0x2A, 0x66, 0xEC, 0xD0,
            0x12, 0x77, 0x30, 0x62, 0xCF, 0x3F, 0xA2, 0x54,
            0x9E, 0x44, 0xF5, 0x8E, 0xD2, 0x40, 0x17, 0x10
        };
        const unsigned char pk[32] = {
            0x25, 0xD1, 0xDF, 0xF9, 0x51, 0x05, 0xF5, 0x25,
            0x3C, 0x40, 0x22, 0xF6, 0x28, 0xA9, 0x96, 0xAD,
            0x3A, 0x0D, 0x95, 0xFB, 0xF2, 0x1D, 0x46, 0x8A,
            0x1B, 0x33, 0xF8, 0xC1, 0x60, 0xD8, 0xF5, 0x17
        };
        const unsigned char msg[32] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };
        const unsigned char sig[64] = {
            0x8B, 0xD2, 0xC1, 0x16, 0x04, 0xB0, 0xA8, 0x7A,
            0x44, 0x3F, 0xCC, 0x2E, 0x5D, 0x90, 0xE5, 0x32,
            0x8F, 0x93, 0x41, 0x61, 0xB1, 0x88, 0x64, 0xFB,
            0x48, 0xCE, 0x10, 0xCB, 0x59, 0xB4, 0x5F, 0xB9,
            0xB5, 0xB2, 0xA0, 0xF1, 0x29, 0xBD, 0x88, 0xF5,
            0xBD, 0xC0, 0x5D, 0x5C, 0x21, 0xE5, 0xC5, 0x71,
            0x76, 0xB9, 0x13, 0x00, 0x23, 0x35, 0x78, 0x4F,
            0x97, 0x77, 0xA2, 0x4B, 0xD3, 0x17, 0xCD, 0x36
        };
        test_schnorrsig_bip_vectors_check_signing(sk, pk, msg, sig);
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 1);
    }
    {
        /* Test vector 4 */
        const unsigned char pk[32] = {
            0xD6, 0x9C, 0x35, 0x09, 0xBB, 0x99, 0xE4, 0x12,
            0xE6, 0x8B, 0x0F, 0xE8, 0x54, 0x4E, 0x72, 0x83,
            0x7D, 0xFA, 0x30, 0x74, 0x6D, 0x8B, 0xE2, 0xAA,
            0x65, 0x97, 0x5F, 0x29, 0xD2, 0x2D, 0xC7, 0xB9
        };
        const unsigned char msg[32] = {
            0x4D, 0xF3, 0xC3, 0xF6, 0x8F, 0xCC, 0x83, 0xB2,
            0x7E, 0x9D, 0x42, 0xC9, 0x04, 0x31, 0xA7, 0x24,
            0x99, 0xF1, 0x78, 0x75, 0xC8, 0x1A, 0x59, 0x9B,
            0x56, 0x6C, 0x98, 0x89, 0xB9, 0x69, 0x67, 0x03
        };
        const unsigned char sig[64] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x3B, 0x78, 0xCE, 0x56, 0x3F,
            0x89, 0xA0, 0xED, 0x94, 0x14, 0xF5, 0xAA, 0x28,
            0xAD, 0x0D, 0x96, 0xD6, 0x79, 0x5F, 0x9C, 0x63,
            0xEE, 0x37, 0x4A, 0xC7, 0xFA, 0xE9, 0x27, 0xD3,
            0x34, 0xCC, 0xB1, 0x90, 0xF6, 0xFB, 0x8F, 0xD2,
            0x7A, 0x2D, 0xDC, 0x63, 0x9C, 0xCE, 0xE4, 0x6D,
            0x43, 0xF1, 0x13, 0xA4, 0x03, 0x5A, 0x2C, 0x7F
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 1);
    }
    {
        /* Test vector 5 */
        const unsigned char pk[32] = {
            0xEE, 0xFD, 0xEA, 0x4C, 0xDB, 0x67, 0x77, 0x50,
            0xA4, 0x20, 0xFE, 0xE8, 0x07, 0xEA, 0xCF, 0x21,
            0xEB, 0x98, 0x98, 0xAE, 0x79, 0xB9, 0x76, 0x87,
            0x66, 0xE4, 0xFA, 0xA0, 0x4A, 0x2D, 0x4A, 0x34
        };
        secp256k1_xonly_pubkey pk_parsed;
        /* No need to check the signature of the test vector as parsing the pubkey already fails */
        CHECK(!secp256k1_xonly_pubkey_parse(ctx, &pk_parsed, pk));
    }
    {
        /* Test vector 6 */
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x13, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0xF9, 0x30, 0x8A, 0x01, 0x92, 0x58, 0xC3, 0x10,
            0x49, 0x34, 0x4F, 0x85, 0xF8, 0x9D, 0x52, 0x29,
            0xB5, 0x31, 0xC8, 0x45, 0x83, 0x6F, 0x99, 0xB0,
            0x86, 0x01, 0xF1, 0x13, 0xBC, 0xE0, 0x36, 0xF9,
            0x93, 0x55, 0x54, 0xD1, 0xAA, 0x5F, 0x03, 0x74,
            0xE5, 0xCD, 0xAA, 0xCB, 0x39, 0x25, 0x03, 0x5C,
            0x7C, 0x16, 0x9B, 0x27, 0xC4, 0x42, 0x6D, 0xF0,
            0xA6, 0xB1, 0x9A, 0xF3, 0xBA, 0xEA, 0xB1, 0x38
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 0);
    }
    {
        /* Test vector 7 */
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x13, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0x10, 0xAC, 0x49, 0xA6, 0xA2, 0xEB, 0xF6, 0x04,
            0x18, 0x9C, 0x5F, 0x40, 0xFC, 0x75, 0xAF, 0x2D,
            0x42, 0xD7, 0x7D, 0xE9, 0xA2, 0x78, 0x27, 0x09,
            0xB1, 0xEB, 0x4E, 0xAF, 0x1C, 0xFE, 0x91, 0x08,
            0xD7, 0x00, 0x3B, 0x70, 0x3A, 0x34, 0x99, 0xD5,
            0xE2, 0x95, 0x29, 0xD3, 0x9B, 0xA0, 0x40, 0xA4,
            0x49, 0x55, 0x12, 0x71, 0x40, 0xF8, 0x1A, 0x8A,
            0x89, 0xA9, 0x6F, 0x99, 0x2A, 0xC0, 0xFE, 0x79
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 0);
    }
    {
        /* Test vector 8 */
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x13, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0x66, 0x7C, 0x2F, 0x77, 0x8E, 0x06, 0x16, 0xE6,
            0x11, 0xBD, 0x0C, 0x14, 0xB8, 0xA6, 0x00, 0xC5,
            0x88, 0x45, 0x51, 0x70, 0x1A, 0x94, 0x9E, 0xF0,
            0xEB, 0xFD, 0x72, 0xD4, 0x52, 0xD6, 0x4E, 0x84,
            0xBE, 0x9F, 0x43, 0x03, 0xC0, 0xB9, 0x91, 0x34,
            0x70, 0x53, 0x2E, 0x65, 0x21, 0xA8, 0x27, 0x95,
            0x1D, 0x39, 0xF5, 0xC6, 0x31, 0xCF, 0xD9, 0x8C,
            0xE3, 0x9A, 0xC4, 0xD7, 0xA5, 0xA8, 0x3B, 0xA9
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 0);
    }
    {
        /* Test vector 9 */
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x13, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x99, 0xD2, 0xF0, 0xEB, 0xC2, 0x99, 0x68, 0x08,
            0x20, 0x86, 0x33, 0xCD, 0x99, 0x26, 0xBF, 0x7E,
            0xC3, 0xDA, 0xB7, 0x3D, 0xAA, 0xD3, 0x6E, 0x85,
            0xB3, 0x04, 0x0A, 0x69, 0x8E, 0x6D, 0x1C, 0xE0
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 0);
    }
    {
        /* Test vector 10 */
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x13, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
            0x24, 0xE8, 0x1D, 0x89, 0xF0, 0x13, 0x04, 0x69,
            0x5C, 0xE9, 0x43, 0xF7, 0xD5, 0xEB, 0xD0, 0x0E,
            0xF7, 0x26, 0xA0, 0x86, 0x4B, 0x4F, 0xF3, 0x38,
            0x95, 0xB4, 0xE8, 0x6B, 0xEA, 0xDC, 0x54, 0x56
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 0);
    }
    {
        /* Test vector 11 */
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x13, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0x4A, 0x29, 0x8D, 0xAC, 0xAE, 0x57, 0x39, 0x5A,
            0x15, 0xD0, 0x79, 0x5D, 0xDB, 0xFD, 0x1D, 0xCB,
            0x56, 0x4D, 0xA8, 0x2B, 0x0F, 0x26, 0x9B, 0xC7,
            0x0A, 0x74, 0xF8, 0x22, 0x04, 0x29, 0xBA, 0x1D,
            0x41, 0x60, 0xBC, 0xFC, 0x3F, 0x46, 0x6E, 0xCB,
            0x8F, 0xAC, 0xD1, 0x9A, 0xDE, 0x57, 0xD8, 0x69,
            0x9D, 0x74, 0xE7, 0x20, 0x7D, 0x78, 0xC6, 0xAE,
            0xDC, 0x37, 0x99, 0xB5, 0x2A, 0x8E, 0x05, 0x98
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 0);
    }
    {
        /* Test vector 12 */
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x13, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFC, 0x2F,
            0x41, 0x60, 0xBC, 0xFC, 0x3F, 0x46, 0x6E, 0xCB,
            0x8F, 0xAC, 0xD1, 0x9A, 0xDE, 0x57, 0xD8, 0x69,
            0x9D, 0x74, 0xE7, 0x20, 0x7D, 0x78, 0xC6, 0xAE,
            0xDC, 0x37, 0x99, 0xB5, 0x2A, 0x8E, 0x05, 0x98
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 0);
    }
    {
        /* Test vector 13 */
        const unsigned char pk[32] = {
            0xDF, 0xF1, 0xD7, 0x7F, 0x2A, 0x67, 0x1C, 0x5F,
            0x36, 0x18, 0x37, 0x26, 0xDB, 0x23, 0x41, 0xBE,
            0x58, 0xFE, 0xAE, 0x1D, 0xA2, 0xDE, 0xCE, 0xD8,
            0x43, 0x24, 0x0F, 0x7B, 0x50, 0x2B, 0xA6, 0x59
        };
        const unsigned char msg[32] = {
            0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3,
            0x14, 0x19, 0x8A, 0x2E, 0x03, 0x70, 0x73, 0x44,
            0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
            0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89
        };
        const unsigned char sig[64] = {
            0x66, 0x7C, 0x2F, 0x77, 0x8E, 0x06, 0x16, 0xE6,
            0x11, 0xBD, 0x0C, 0x14, 0xB8, 0xA6, 0x00, 0xC5,
            0x88, 0x45, 0x51, 0x70, 0x1A, 0x94, 0x9E, 0xF0,
            0xEB, 0xFD, 0x72, 0xD4, 0x52, 0xD6, 0x4E, 0x84,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
            0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
            0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41
        };
        test_schnorrsig_bip_vectors_check_verify(scratch, pk, msg, sig, 0);
    }
    {
        /* Test vector 14 */
        const unsigned char pk[32] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFC, 0x30
        };
        secp256k1_xonly_pubkey pk_parsed;
        /* No need to check the signature of the test vector as parsing the pubkey already fails */
        CHECK(!secp256k1_xonly_pubkey_parse(ctx, &pk_parsed, pk));
    }
}

/* Nonce function that returns constant 0 */
static int nonce_function_failing(unsigned char *nonce32, const unsigned char *msg32, const unsigned char *key32, const unsigned char *algo16, void *data, unsigned int counter) {
    (void) msg32;
    (void) key32;
    (void) algo16;
    (void) data;
    (void) counter;
    (void) nonce32;
    return 0;
}

/* Nonce function that sets nonce to 0 */
static int nonce_function_0(unsigned char *nonce32, const unsigned char *msg32, const unsigned char *key32, const unsigned char *algo16, void *data, unsigned int counter) {
    (void) msg32;
    (void) key32;
    (void) algo16;
    (void) data;
    (void) counter;

    memset(nonce32, 0, 32);
    return 1;
}

void test_schnorrsig_sign(void) {
    unsigned char sk[32];
    const unsigned char msg[32] = "this is a msg for a schnorrsig..";
    secp256k1_schnorrsig sig;
    unsigned char zeros64[64];


    memset(sk, 23, sizeof(sk));
    CHECK(secp256k1_schnorrsig_sign(ctx, &sig, msg, sk, NULL, NULL) == 1);

    memset(zeros64, 0, 64);
    /* Overflowing secret key */
    memset(sk, 0xFF, sizeof(sk));
    CHECK(secp256k1_schnorrsig_sign(ctx, &sig, msg, sk, NULL, NULL) == 0);
    CHECK(memcmp(&sig, zeros64, sizeof(sig)) == 0);

    /* Zeroed secret key */
    memset(sk, 0, sizeof(sk));
    memset(&sig, 1, sizeof(sig));
    CHECK(secp256k1_schnorrsig_sign(ctx, &sig, msg, sk, NULL, NULL) == 0);
    CHECK(memcmp(&sig, zeros64, sizeof(sig)) == 0);

    memset(sk, 23, sizeof(sk));
    memset(&sig, 1, sizeof(sig));
    CHECK(secp256k1_schnorrsig_sign(ctx, &sig, msg, sk, nonce_function_failing, NULL) == 0);
    CHECK(memcmp(&sig, zeros64, sizeof(sig)) == 0);
    memset(&sig, 1, sizeof(sig));
    CHECK(secp256k1_schnorrsig_sign(ctx, &sig, msg, sk, nonce_function_0, NULL) == 0);
    CHECK(memcmp(&sig, zeros64, sizeof(sig)) == 0);
}

#define N_SIGS  200
/* Creates N_SIGS valid signatures and verifies them with verify and verify_batch. Then flips some
 * bits and checks that verification now fails. */
void test_schnorrsig_sign_verify(secp256k1_scratch_space *scratch) {
    const unsigned char sk[32] = "shhhhhhhh! this key is a secret.";
    unsigned char msg[N_SIGS][32];
    secp256k1_schnorrsig sig[N_SIGS];
    size_t i;
    const secp256k1_schnorrsig *sig_arr[N_SIGS];
    const unsigned char *msg_arr[N_SIGS];
    const secp256k1_xonly_pubkey *pk_arr[N_SIGS];
    secp256k1_xonly_pubkey pk;

    CHECK(secp256k1_xonly_pubkey_create(ctx, &pk, sk));

    CHECK(secp256k1_schnorrsig_verify_batch(ctx, scratch, NULL, NULL, NULL, 0));

    for (i = 0; i < N_SIGS; i++) {
        secp256k1_rand256(msg[i]);
        CHECK(secp256k1_schnorrsig_sign(ctx, &sig[i], msg[i], sk, NULL, NULL));
        CHECK(secp256k1_schnorrsig_verify(ctx, &sig[i], msg[i], &pk));
        sig_arr[i] = &sig[i];
        msg_arr[i] = msg[i];
        pk_arr[i] = &pk;
    }

    CHECK(secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, 1));
    CHECK(secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, 2));
    CHECK(secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, 4));
    CHECK(secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, N_SIGS));

    {
        /* Flip a few bits in the signature and in the message and check that
         * verify and verify_batch fail */
        size_t sig_idx = secp256k1_rand_int(4);
        size_t byte_idx = secp256k1_rand_int(32);
        unsigned char xorbyte = secp256k1_rand_int(254)+1;
        sig[sig_idx].data[byte_idx] ^= xorbyte;
        CHECK(!secp256k1_schnorrsig_verify(ctx, &sig[sig_idx], msg[sig_idx], &pk));
        CHECK(!secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, 4));
        sig[sig_idx].data[byte_idx] ^= xorbyte;

        byte_idx = secp256k1_rand_int(32);
        sig[sig_idx].data[32+byte_idx] ^= xorbyte;
        CHECK(!secp256k1_schnorrsig_verify(ctx, &sig[sig_idx], msg[sig_idx], &pk));
        CHECK(!secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, 4));
        sig[sig_idx].data[32+byte_idx] ^= xorbyte;

        byte_idx = secp256k1_rand_int(32);
        msg[sig_idx][byte_idx] ^= xorbyte;
        CHECK(!secp256k1_schnorrsig_verify(ctx, &sig[sig_idx], msg[sig_idx], &pk));
        CHECK(!secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, 4));
        msg[sig_idx][byte_idx] ^= xorbyte;

        /* Check that above bitflips have been reversed correctly */
        CHECK(secp256k1_schnorrsig_verify(ctx, &sig[sig_idx], msg[sig_idx], &pk));
        CHECK(secp256k1_schnorrsig_verify_batch(ctx, scratch, sig_arr, msg_arr, pk_arr, 4));
    }
}
#undef N_SIGS

void test_schnorrsig_taproot(void) {
    unsigned char sk[32];
    secp256k1_xonly_pubkey internal_pk;
    unsigned char internal_pk_bytes[32];
    secp256k1_xonly_pubkey output_pk;
    unsigned char output_pk_bytes[32];
    unsigned char tweak[32];
    int is_negated;
    unsigned char msg[32];
    secp256k1_schnorrsig sig;

    /* Create output key */
    secp256k1_rand256(sk);
    CHECK(secp256k1_xonly_pubkey_create(ctx, &internal_pk, sk) == 1);
    memset(tweak, 1, sizeof(tweak));
    CHECK(secp256k1_xonly_pubkey_tweak_add(ctx, &output_pk, &is_negated, &internal_pk, tweak) == 1);
    CHECK(secp256k1_xonly_pubkey_serialize(ctx, output_pk_bytes, &output_pk) == 1);

    /* Key spend */
    secp256k1_rand256(msg);
    CHECK(secp256k1_xonly_privkey_tweak_add(ctx, sk, tweak) == 1);
    CHECK(secp256k1_schnorrsig_sign(ctx, &sig, msg, sk, NULL, NULL) == 1);
    /* Verify key spend */
    CHECK(secp256k1_xonly_pubkey_parse(ctx, &output_pk, output_pk_bytes) == 1);
    CHECK(secp256k1_schnorrsig_verify(ctx, &sig, msg, &output_pk) == 1);

    /* Script spend */
    CHECK(secp256k1_xonly_pubkey_serialize(ctx, internal_pk_bytes, &internal_pk) == 1);
    /* Verify script spend */
    CHECK(secp256k1_xonly_pubkey_parse(ctx, &output_pk, output_pk_bytes) == 1);
    CHECK(secp256k1_xonly_pubkey_parse(ctx, &internal_pk, internal_pk_bytes) == 1);
    CHECK(secp256k1_xonly_pubkey_tweak_test(ctx, &output_pk, is_negated, &internal_pk, tweak) == 1);
}

void run_schnorrsig_tests(void) {
    secp256k1_scratch_space *scratch = secp256k1_scratch_space_create(ctx, 1024 * 1024);

    test_schnorrsig_serialize();
    test_schnorrsig_api(scratch);
    test_schnorrsig_sha256_tagged();
    test_schnorrsig_bip_vectors(scratch);
    test_schnorrsig_sign();
    test_schnorrsig_sign_verify(scratch);
    test_schnorrsig_taproot();

    secp256k1_scratch_space_destroy(ctx, scratch);
}

#endif
