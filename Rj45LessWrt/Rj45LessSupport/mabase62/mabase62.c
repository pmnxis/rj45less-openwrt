/*
 * Copyright (C) 2021 Jinwoo Park <pmnxis@gmail.com>
 * Released under the terms of the GNU GPL v3.0.
 * mabase62.c - Mangle some stdin base16 strings to customized base62.
 * customized base62 include numeric digits, upper and lower cases alphabet.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum mabase62_mode_t
{
    mabase62_mode_normal,
    mabase62_mode_less,
    mabase62_mode_greater,
    mabase62_mode_zero,
} mabase62_mode_t;

static const char __mabase62_zerofill[] = "Invalid0Invalid0Invalid0Invalid0";

static bool Check_09AZaz(uint8_t data)
{
    if ((('A' <= data) && (data <= 'Z')) ||
        (('a' <= data) && (data <= 'z')) ||
        (('0' <= data) && (data <= '9')))
        return true;
    else
        return false;
}

static uint8_t mangle_u8(uint8_t data)
{
    uint8_t temp;
    temp = data & 0x7F;
    if (Check_09AZaz(temp))
        return temp;
    temp = (data >> 1) & 0x3F;
    if (temp < 10)
        return temp + '0';
    else if (temp < (10 + 26))
        return temp - 10 + 'A';
    else if (temp < (10 + 26 + 26))
        return temp - 10 - 26 + 'a';
    // not reachable
    else
        return '0';
}

static uint8_t __base16_to_u4(char s)
{
    uint8_t temp;
    if ('0' <= s && s <= '9')
        return (uint8_t)(s - '0');
    temp = (uint8_t)s | 0x60;
    if ('f' < temp)
        return 0;
    else
        return (temp - 'a' + 10);
}

/**
 * __mabase62 - Mangle 256bit data to custom CT62 standard ascii data.
 * @dst: data should be uint8_t[32]
 * @src: source
 */
static void __mabase62(uint8_t *dst, uint8_t *src)
{
    for (int i = 0; i < 32; i++)
    {
        uint8_t sss = (__base16_to_u4(src[(i << 1) + 1])) |
                      (__base16_to_u4(src[(i << 1) + 0]) << 4);
        dst[i] = mangle_u8(sss);
    }
}

/**
 * __mabase62_lt - Mangle 8~248bit data to custom CT62 standard ascii data.
 * @dst: data should be uint8_t[32]
 * @src: source
 * @len: length of data
 */
static void __mabase62_lt(uint8_t *dst, uint8_t *src, int len)
{
    int gap = 32 - (len >> 1);
    for (int i = 0; i < gap; i++)
    {
        dst[i] = mangle_u8(0);
    }

    for (int i = gap; i < 32; i++)
    {
        uint8_t sss = (__base16_to_u4(src[((i - gap) << 1) + 1])) |
                      (__base16_to_u4(src[((i - gap) << 1) + 0]) << 4);
        dst[i] = mangle_u8(sss);
    }
}

/**
 * __mabase62_gt - Mangle 264~2048bit data to custom CT62 standard ascii data.
 * if larger than 2048, cut off.
 * @dst: data should be uint8_t[32]
 * @src: source
 * @len: length of data
 */
static void __mabase62_gt(uint8_t *dst, uint8_t *src, int len)
{
    int barrier_len = ((len >> 1) < 256) ? (len >> 1) : 256;
    for (int i = 0; i < barrier_len; i++)
    {
        uint8_t sss = (__base16_to_u4(src[(i << 1) + 1])) |
                      (__base16_to_u4(src[(i << 1) + 0]) << 4);
        dst[i & 31] = mangle_u8(src[i]);
    }
}

/**
 * __mabase62_zerofill - Dummy Mangler.
 * @dst: data should be uint8_t[32]
 */
static void __mabase62_zerofill(uint8_t *dst)
{
    for (int i = 0; i < 32; i++)
    {
        dst[i] = (uint8_t)__mabase62_zerofill[i];
    }
}

int main(int argc, char *argv[])
{
    mabase62_mode_t mode = mabase62_mode_zero;
    uint8_t output[33] = {
        0,
    };
    int len_input;

    if (argc != 2)
        mode = mabase62_mode_zero;
    else
    {
        len_input = strlen(argv[1]);

        if (len_input == 64)
            mode = mabase62_mode_normal;
        else if (len_input == 0)
            mode = mabase62_mode_zero;
        else if (len_input < 64)
            mode = mabase62_mode_less;
        else if (64 < len_input)
            mode = mabase62_mode_greater;
        else
            mode = mabase62_mode_zero;
    }

    switch (mode)
    {
    case mabase62_mode_normal:
        __mabase62(output, (uint8_t *)argv[1]);
        break;
    case mabase62_mode_less:
        __mabase62_lt(output, (uint8_t *)argv[1], len_input);
        break;
    case mabase62_mode_greater:
        __mabase62_gt(output, (uint8_t *)argv[1], len_input);
        break;
    case mabase62_mode_zero:
    default:
        __mabase62_zerofill(output);
        break;
    }

    output[32] = 0;
    puts((char *)output);

    return 0;
}