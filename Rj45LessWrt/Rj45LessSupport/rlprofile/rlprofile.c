/*
 * Copyright (C) 2021 Jinwoo Park <pmnxis@gmail.com>
 * Released under the terms of the GNU GPL v3.0.
 * rlprofile.c - rj45less profile control by stdio.
 * Currently this code is only suitable to IPTIME-EXTN3 openwrt port.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

enum
{
    some_delay_time_us = 10,
    profile_address = 0xFF00,
};

const char rlprofile_magic_str[] = "RJ45LESS_V1_PRF_";

typedef struct
{
    uint8_t magic[16];
    uint8_t mid_primary[4];
    uint8_t mid_secondary[4];
    uint8_t dummy[8];
    uint8_t hash[32];
} rlprofile_t;

const char target_mtd_str[] = "/dev/mtd1";
const char sha_buffer_src[] = "/tmp/sha_src.bin";
const char sha_command[] = "/usr/bin/sha256sum /tmp/sha_src.bin";
const char my_help[] = "Usage: rlprofile [function...] [extra...]...\nRj45Less WiFi MID Profile Utilities.\nCheck Rj45Less config profile in SPI-ROM.\n\nMandatory arguments to long options are mandatory for short options too.\n\n -c,  --check-profile         Validate WRT config in ROM, then return MID.\n -w,  --write-profile <MID>   Write config in ROM with given MID.\n -e,  --erase-profile         Erase config partition in ROM.\n";


/* Pass acutal and expect are equal, else exit. */
static void fcheck(const char *comment, int actual, int expect)
{
    if (expect == actual)
    {
        printf("FAIL_%s %d %d\n", comment, expect, actual);
        usleep(some_delay_time_us);
        _exit(0);
    }
}

/* Pass acutal and expect are not equal, else exit. */
static void fcheck_ne(const char *comment, int actual, int expect)
{
    if (expect != actual)
    {
        printf("FAIL_%s %d %d\n", comment, expect, actual);
        usleep(some_delay_time_us);
        _exit(0);
    }
}

/* Pass acutal is less than expect, else exit. */
static void fcheck_lt(const char *comment, int actual, int expect)
{
    if (actual < expect)
    {
        printf("FAIL%s %d %d\n", comment, expect, actual);
        usleep(some_delay_time_us);
        _exit(0);
    }
}

/* Pass acutal and expect are equal, else exit with closing file descriptor. */
static void fcheck_wclose(const char *comment, int actual, int expect, int fd)
{
    if (actual == expect)
    {
        printf("FAIL%s %d %d\n", comment, expect, actual);
        usleep(some_delay_time_us);
        close(fd);
        _exit(0);
    }
}

/* Pass acutal is less than expect, else exit with closing file descriptor. */
static void fcheck_lt_wclose(const char *comment, int actual, int expect, int fd)
{
    if (actual < expect)
    {
        printf("FAIL%s %d %d\n", comment, expect, actual);
        usleep(some_delay_time_us);
        close(fd);
        _exit(0);
    }
}

/* Single base16 based char to half-width of uint8_t */
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

/* write 32bit data as little endian */
static int write_le_u32(uint32_t src, uint8_t *dst)
{
    if (dst == 0)
    {
        return -2;
    }
    dst[0] = (uint8_t)(src & 0xFF);
    dst[1] = (uint8_t)((src >> 8) & 0xFF);
    dst[2] = (uint8_t)((src >> 16) & 0xFF);
    dst[3] = (uint8_t)((src >> 24) & 0xFF);
    return 0;
}

/* read little endianed 4byte data */
static int read_le_u32(uint8_t *src, uint32_t *dst)
{
    uint32_t temp = 0;
    if (dst == 0 || src == 0)
        return -2;
    *dst = src[0] + (src[1] << 8) + (src[2] << 16) + (src[3] << 24);
    return 0;
}

/* read mesh id from profile structure, if there's problem return error */
static int32_t get_mid(rlprofile_t *profile)
{
    int ret = 0;
    uint32_t left;
    uint32_t right;

    ret |= read_le_u32(profile->mid_primary, &left);
    ret |= read_le_u32(profile->mid_secondary, &right);

    if (ret < 0)
        return -2;
    if (left == right && 100000 <= left && left <= 899999)
        return left;
    else
        return -1;
}

/* char string to mid (6digits) */
static int32_t check_from_mid_string(char *str)
{
    int32_t temp = 0;
    for (char *cc = str, i = 0; *cc != 0; cc++, i++)
    {
        char tt = *cc;
        if (('0' <= tt && tt <= '9'))
        {
            if (i >= 6)
            {
                return -1;
            }
            temp = temp * 10 + (int32_t)(tt - '0');
        }
        else if (i >= 6 && (tt == ' ' || tt == 0xd || tt == 0xa))
        {
            return temp;
        }
        else
        {
            return -2;
        }
    }
    return temp;
}

/* calculate sha256 by openwrt internal sha256 device 
   this approach looks dirty, but for reduce rom memory */
static int calc_sha256(int32_t mid, uint8_t *dst)
{
    int fd;
    int ret;
    FILE *sha_result;
    pid_t pid;
    char bump[32];
    if (access(sha_buffer_src, F_OK) == 0)
    {
        ret = remove(sha_buffer_src);
        fcheck_lt("cs0", ret, 0);
    }

    snprintf(bump, 32, "Rj45LessProfile-%06dEE", mid);

    fd = open(sha_buffer_src, O_RDWR | O_CREAT, 0660);
    fcheck_lt("cs1", fd, 0);

    ret = write(fd, bump, strlen(bump));
    fcheck_lt_wclose("cs2", ret, 0, fd);

    close(fd);

    sha_result = popen(sha_command, "r");
    char buffer[128];
    fscanf(sha_result, "%64s", buffer);

    pclose(sha_result);

    ret = remove(sha_buffer_src);
    fcheck_lt("cs3", ret, 0);

    for (int i = 0; i < 32; i++)
    {
        dst[i] = (__base16_to_u4(buffer[(i << 1) + 1])) |
                 (__base16_to_u4(buffer[(i << 1) + 0]) << 4);
    }

    return 0;
}

static int check_profile()
{
    int fd;
    int ret;
    int32_t mid;
    rlprofile_t profile;
    uint8_t correct_hash[32];

    fd = open(target_mtd_str, O_RDONLY);
    fcheck_lt("cp0", fd, 0);

    ret = lseek(fd, profile_address, SEEK_SET);
    fcheck_lt_wclose("cp1", ret, 0, fd);

    ret = read(fd, &profile, sizeof(rlprofile_t));
    fcheck_lt_wclose("cp2", ret, 0, fd);

    close(fd);

    mid = get_mid(&profile);
    fcheck("cp3", mid, -2);
    fcheck("cp4", mid, -1);

    calc_sha256(mid, correct_hash);
    ret = memcmp(correct_hash, profile.hash, 32);
    fcheck_ne("cp5", ret, 0);

    return mid;
}

static int write_profile(int32_t mid)
{
    mtd_info_t mtd_info;
    erase_info_t ei;
    rlprofile_t profile;
    rlprofile_t profile_doublecheck;
    int fd;
    int ret;
    unsigned int end;

    memcpy(profile.magic, rlprofile_magic_str, 16);
    write_le_u32(mid, profile.mid_primary);
    write_le_u32(mid, profile.mid_secondary);
    memset(profile.dummy, 0x00, 8);
    calc_sha256(mid, profile.hash);

    fd = open(target_mtd_str, O_RDWR);
    fcheck_lt("wp0", fd, 0);

    ret = ioctl(fd, MEMGETINFO, &mtd_info); // get the device info
    fcheck_lt_wclose("wp1", ret, 0, fd);

    ei.length = mtd_info.erasesize; //set the erase block size
    ei.start = (profile_address / ei.length) * ei.length;
    end = profile_address + sizeof(rlprofile_t);
    end = (end % ei.length) == 0
              ? (end / ei.length) * ei.length
              : ((end / ei.length) + 1) * (ei.length);

    for (; ei.start < end; ei.start += ei.length)
    {
        ioctl(fd, MEMUNLOCK, &ei);
        // warning, this prints a lot!
        ioctl(fd, MEMERASE, &ei);
    }

    ret = lseek(fd, profile_address, SEEK_SET);
    fcheck_lt_wclose("wp2", ret, 0, fd);

    ret = write(fd, &profile, sizeof(rlprofile_t));
    fcheck_lt_wclose("wp3", ret, 0, fd);

    ret = lseek(fd, profile_address, SEEK_SET);
    fcheck_lt_wclose("wp4", ret, 0, fd);

    ret = read(fd, &profile_doublecheck, sizeof(rlprofile_t));
    fcheck_lt_wclose("wp5", ret, 0, fd);

    close(fd);

    return (memcmp(&profile_doublecheck, &profile, sizeof(rlprofile_t)) == 0)
        ? 0 : -1;
}

static int erase_config()
{
    mtd_info_t mtd_info;
    erase_info_t ei;
    rlprofile_t dummy;
    int fd;
    int ret;
    unsigned int end;

    fd = open(target_mtd_str, O_RDWR);
    fcheck_lt("er0", fd, 0);

    ret = ioctl(fd, MEMGETINFO, &mtd_info); // get the device info
    fcheck_lt_wclose("er1", ret, 0, fd);

    ei.length = mtd_info.erasesize; //set the erase block size
    ei.start = (profile_address / ei.length) * ei.length;
    end = profile_address + sizeof(rlprofile_t);
    end = (end % ei.length) == 0
              ? (end / ei.length) * ei.length
              : ((end / ei.length) + 1) * (ei.length);

    for (; ei.start < end; ei.start += ei.length)
    {
        ioctl(fd, MEMUNLOCK, &ei);
        ioctl(fd, MEMERASE, &ei);
    }

    ret = lseek(fd, profile_address, SEEK_SET);
    fcheck_lt_wclose("er2", ret, 0, fd);

    ret = read(fd, &dummy, sizeof(rlprofile_t));
    fcheck_lt_wclose("er3", ret, 0, fd);

    close(fd);

    ret = 0;
    uint8_t *__read_buf = (uint8_t *)&dummy;
    for (int i = 0; i < sizeof(rlprofile_t); i++)
    {
        fcheck_ne(
            "er4",
            (__read_buf[i] == 0x00 || __read_buf[i] == 0xFF),
            1);
    }
    return 0;
}

static int find_parm(int argc, char *argv[], const char *arr)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], arr) == 0)
            return i;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int ccs = find_parm(argc, argv, "-c");
    int ccl = find_parm(argc, argv, "--check-profile");
    int wps = find_parm(argc, argv, "-w");
    int wpl = find_parm(argc, argv, "--write-profile");
    int ers = find_parm(argc, argv, "-e");
    int erl = find_parm(argc, argv, "--erase-profile");

    uint8_t parm_flag = ((ccs != 0) ^ (ccl != 0)) +
                        ((wps != 0) ^ (wpl != 0)) +
                        ((ers != 0) ^ (erl != 0));

    if (parm_flag != 1)
    {
        printf("FAIL_ARG unrecognized option\n");
        puts(my_help);
    }
    else if ((ccs != 0) ^ (ccl != 0))
    {
        // Read Profile
        printf("OK %06d\n", check_profile());
    }
    else if ((wps != 0) ^ (wpl != 0))
    {
        // Write Profile
        wps |= wpl;
        int32_t mid = check_from_mid_string(argv[wps + 1]);
        // 0xxxxx 9xxxxx uses for developer side. you can modify this.
        if (mid < 100000 || 899999 < mid)
        {
            printf("FAIL_ARG unrecognized option\n");
            printf("unrecognized option on MID\n100000 <= MID <= 899999\n");
            printf("Your MID Input : %d\n", mid);
            return 0;
        }
        write_profile(mid);
        printf("OK %06d\n", mid);
    }
    else if ((ers != 0) ^ (erl != 0))
    {
        // Erase Profile
        erase_config();
        printf("OK\n");
    }
}
