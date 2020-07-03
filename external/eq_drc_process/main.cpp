#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <errno.h>

#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "Rk_wake_lock.h"
#include "Rk_socket_app.h"

#define SAMPLE_RATE 48000
#define CHANNEL 2
#define REC_DEVICE_NAME "fake_record"
#define WRITE_DEVICE_NAME "fake_play"
#define JACK_DEVICE_NAME "fake_jack"
#define READ_FRAME  1024    //(768)
#define PERIOD_SIZE (1024)  //(SAMPLE_RATE/8)
#define PERIOD_counts (20) //double of delay 200ms
#define BUFFER_SIZE (PERIOD_SIZE * PERIOD_counts)
#define MUTE_TIME_THRESHOD (4)//seconds
#define MUTE_FRAME_THRESHOD (SAMPLE_RATE * MUTE_TIME_THRESHOD / READ_FRAME)//30 seconds
//#define ALSA_READ_FORMAT SND_PCM_FORMAT_S32_LE
#define ALSA_READ_FORMAT SND_PCM_FORMAT_S16_LE
#define ALSA_WRITE_FORMAT SND_PCM_FORMAT_S16_LE

/*
 * Select different alsa pathways based on device type.
 *  LINE_OUT: LR-Mix(fake_play)->EqDrcProcess(ladspa)->Speaker(real_playback)
 *  HEAD_SET: fake_jack -> Headset(real_playback)
 *  BLUETOOTH: device as bluetooth source.
 */
#define DEVICE_FLAG_LINE_OUT        0x01
#define DEVICE_FLAG_HEAD_SET        0x02
#define DEVICE_FLAG_BLUETOOTH       0x04
#define DEVICE_FLAG_BLUETOOTH_BSA   0x05

enum BT_CONNECT_STATE{
    BT_DISCONNECT = 0,
    BT_CONNECT_BLUEZ,
    BT_CONNECT_BSA
};

static char g_bt_mac_addr[17];
static enum BT_CONNECT_STATE g_bt_is_connect = BT_DISCONNECT;
static bool g_system_sleep = false;
static char sock_path[] = "/data/bsa/config/bsa_socket";

struct timeval tv_begin, tv_end;
//gettimeofday(&tv_begin, NULL);

extern int set_sw_params(snd_pcm_t *pcm, snd_pcm_uframes_t buffer_size,
                         snd_pcm_uframes_t period_size, char **msg);

void alsa_fake_device_record_open(snd_pcm_t** capture_handle,int channels,uint32_t rate)
{
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_uframes_t periodSize = PERIOD_SIZE;
    snd_pcm_uframes_t bufferSize = BUFFER_SIZE;
    int dir = 0;
    int err;

    err = snd_pcm_open(capture_handle, REC_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
    if (err)
    {
        printf( "Unable to open capture PCM device: \n");
        exit(1);
    }
    printf("snd_pcm_open\n");
    //err = snd_pcm_hw_params_alloca(&hw_params);

    err = snd_pcm_hw_params_malloc(&hw_params);
    if(err)
    {
        fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
        exit(1);
    }
    printf("snd_pcm_hw_params_malloc\n");

    err = snd_pcm_hw_params_any(*capture_handle, hw_params);
    if(err)
    {
        fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",snd_strerror(err));
        exit(1);
    }
    printf("snd_pcm_hw_params_any!\n");

    err = snd_pcm_hw_params_set_access(*capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    // err = snd_pcm_hw_params_set_access(*capture_handle, hw_params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
    if (err)
    {
        printf("Error setting interleaved mode\n");
        exit(1);
    }
    printf("snd_pcm_hw_params_set_access!\n");

    err = snd_pcm_hw_params_set_format(*capture_handle, hw_params, ALSA_READ_FORMAT);
    if (err)
    {
        printf("Error setting format: %s\n", snd_strerror(err));
        exit(1);
    }
    printf("snd_pcm_hw_params_set_format\n");

    err = snd_pcm_hw_params_set_channels(*capture_handle, hw_params, channels);
    if (err)
    {
        printf("channels = %d\n",channels);
        printf( "Error setting channels: %s\n", snd_strerror(err));
        exit(1);
    }
    printf("channels = %d\n",channels);

    err = snd_pcm_hw_params_set_buffer_size_near(*capture_handle, hw_params, &bufferSize);
    if (err)
    {
        printf("Error setting buffer size (%d): %s\n", bufferSize, snd_strerror(err));
        exit(1);
    }
    printf("bufferSize = %d\n",bufferSize);

    err = snd_pcm_hw_params_set_period_size_near(*capture_handle, hw_params, &periodSize, 0);
    if (err)
    {
        printf("Error setting period time (%d): %s\n", periodSize, snd_strerror(err));
        exit(1);
    }
    printf("periodSize = %d\n",periodSize);

    err = snd_pcm_hw_params_set_rate_near(*capture_handle, hw_params, &rate, 0/*&dir*/);
    if (err)
    {
        printf("Error setting sampling rate (%d): %s\n", rate, snd_strerror(err));
        //goto error;
        exit(1);
    }
    printf("Rate = %d\n", rate);

    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(*capture_handle, hw_params);
    if (err < 0)
    {
        printf( "Unable to set HW parameters: %s\n", snd_strerror(err));
        //goto error;
        exit(1);
    }

    printf("Open record device done \n");
    //set_sw_params(*capture_handle,bufferSize,periodSize,NULL);
    if(hw_params)
        snd_pcm_hw_params_free(hw_params);
}

void alsa_fake_device_write_open(snd_pcm_t** write_handle, int channels,
                                 uint32_t write_sampleRate, int device_flag,
                                 int *socket_fd)
{
    snd_pcm_hw_params_t *write_params;
    snd_pcm_uframes_t write_periodSize = PERIOD_SIZE;
    snd_pcm_uframes_t write_bufferSize = BUFFER_SIZE;
    int write_err;
    int write_dir;
    char bluealsa_device[256] = {0};

    if (device_flag == DEVICE_FLAG_HEAD_SET) {
        printf("Open PCM: %s\n", JACK_DEVICE_NAME);
        write_err = snd_pcm_open(write_handle, JACK_DEVICE_NAME,
                                 SND_PCM_STREAM_PLAYBACK, 0);
    } else if (device_flag == DEVICE_FLAG_BLUETOOTH) {
        sprintf(bluealsa_device, "%s%s", "bluealsa:HCI=hci0,PROFILE=a2dp,DEV=",
                g_bt_mac_addr);
        printf("Open PCM: %s\n", bluealsa_device);
        write_err = snd_pcm_open(write_handle, bluealsa_device,
                                 SND_PCM_STREAM_PLAYBACK, 0);
    } else if (device_flag == DEVICE_FLAG_BLUETOOTH_BSA) {
        *socket_fd = RK_socket_client_setup(sock_path);
        if (*socket_fd < 0)
            printf("Fail to connect server socket\n");
        else
            printf("Socket client connected\n");

        return;
    } else {
        printf("Open PCM: %s\n", WRITE_DEVICE_NAME);
        write_err = snd_pcm_open(write_handle, WRITE_DEVICE_NAME,
                                 SND_PCM_STREAM_PLAYBACK, 0);
    }

    if (write_err) {
        printf( "Unable to open playback PCM device: \n");
        exit(1);
    }
    printf( "interleaved mode\n");

    // snd_pcm_hw_params_alloca(&write_params);
    snd_pcm_hw_params_malloc(&write_params);
    printf("snd_pcm_hw_params_alloca\n");

    snd_pcm_hw_params_any(*write_handle, write_params);

    write_err = snd_pcm_hw_params_set_access(*write_handle, write_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    //write_err = snd_pcm_hw_params_set_access(*write_handle,  write_params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
    if (write_err)
    {
        printf("Error setting interleaved mode\n");
        exit(1);
    }
    printf( "interleaved mode\n");

    write_err = snd_pcm_hw_params_set_format(*write_handle, write_params, ALSA_WRITE_FORMAT);
    if (write_err)
    {
        printf("Error setting format: %s\n", snd_strerror(write_err));
        exit(1);
    }
    printf( "format successed\n");

    write_err = snd_pcm_hw_params_set_channels(*write_handle, write_params, channels);
    if (write_err)
    {
        printf( "Error setting channels: %s\n", snd_strerror(write_err));
        exit(1);
    }
    printf("channels = %d\n",channels);

    write_err = snd_pcm_hw_params_set_rate_near(*write_handle, write_params, &write_sampleRate, 0/*&write_dir*/);
    if (write_err)
    {
        printf("Error setting sampling rate (%d): %s\n", write_sampleRate, snd_strerror(write_err));
        exit(1);
    }
    printf("setting sampling rate (%d)\n", write_sampleRate);

    write_err = snd_pcm_hw_params_set_buffer_size_near(*write_handle, write_params, &write_bufferSize);
    if (write_err)
    {
        printf("Error setting buffer size (%ld): %s\n", write_bufferSize, snd_strerror(write_err));
        exit(1);
    }
    printf("write_bufferSize = %d\n",write_bufferSize);

    write_err = snd_pcm_hw_params_set_period_size_near(*write_handle, write_params, &write_periodSize, 0);
    if (write_err)
    {
        printf("Error setting period time (%ld): %s\n", write_periodSize, snd_strerror(write_err));
        exit(1);
    }
    printf("write_periodSize = %d\n",write_periodSize);

#if 0
    snd_pcm_uframes_t write_final_buffer;
    write_err = snd_pcm_hw_params_get_buffer_size(write_params, &write_final_buffer);
    printf(" final buffer size %ld \n" , write_final_buffer);

    snd_pcm_uframes_t write_final_period;
    write_err = snd_pcm_hw_params_get_period_size(write_params, &write_final_period, &write_dir);
    printf(" final period size %ld \n" , write_final_period);
#endif

    /* Write the parameters to the driver */
    write_err = snd_pcm_hw_params(*write_handle, write_params);
    if (write_err < 0)
    {
        printf( "Unable to set HW parameters: %s\n", snd_strerror(write_err));
        exit(1);
    }

    printf("open write device is successful\n");
    set_sw_params(*write_handle, write_bufferSize, write_periodSize, NULL);
    if(write_params)
        snd_pcm_hw_params_free(write_params);
}

int set_sw_params(snd_pcm_t *pcm, snd_pcm_uframes_t buffer_size,
                  snd_pcm_uframes_t period_size, char **msg) {

    snd_pcm_sw_params_t *params;
    char buf[256];
    int err;

    //snd_pcm_sw_params_alloca(&params);
    snd_pcm_sw_params_malloc(&params);
    if ((err = snd_pcm_sw_params_current(pcm, params)) != 0) {
        snprintf(buf, sizeof(buf), "Get current params: %s", snd_strerror(err));
        //goto fail;
        exit(1);
    }

    /* start the transfer when the buffer is full (or almost full) */
    snd_pcm_uframes_t threshold = (buffer_size / period_size) * period_size;
    if ((err = snd_pcm_sw_params_set_start_threshold(pcm, params, threshold)) != 0) {
        snprintf(buf, sizeof(buf), "Set start threshold: %s: %lu", snd_strerror(err), threshold);
        exit(1);
    }

    /* allow the transfer when at least period_size samples can be processed */
    if ((err = snd_pcm_sw_params_set_avail_min(pcm, params, period_size)) != 0) {
        snprintf(buf, sizeof(buf), "Set avail min: %s: %lu", snd_strerror(err), period_size);
        exit(1);
    }

    if ((err = snd_pcm_sw_params(pcm, params)) != 0) {
        snprintf(buf, sizeof(buf), "%s", snd_strerror(err));
        exit(1);
    }
    if(params)
        snd_pcm_sw_params_free(params);

    return 0;
}

int is_mute_frame(short *in,unsigned int size)
{
    int i;
    int mute_count = 0;

    if (!size) {
        printf("frame size is zero!!!\n");
        return 0;
    }
    for (i = 0; i < size;i ++) {
        if(in[i] != 0)
        return 0;
    }

    return 1;
}

/* Determine whether to enter the energy saving mode according to
 * the value of the environment variable "EQ_LOW_POWERMODE"
 */
bool low_power_mode_check()
{
    char *value = NULL;

    /* env: "EQ_LOW_POWERMODE=TRUE" or "EQ_LOW_POWERMODE=true" ? */
    value = getenv("EQ_LOW_POWERMODE");
    if (value && (!strcmp("TRUE", value) || !strcmp("true", value)))
        return true;

    return false;
}

/* Check device changing. */
int get_device_flag()
{
    int fd = 0, ret = 0;
    char buff[512] = {0};
    int device_flag = DEVICE_FLAG_LINE_OUT;
    const char *path = "/sys/devices/platform/ff560000.acodec/rk3308-acodec-dev/dac_output";
    FILE *pp = NULL; /* pipeline */
    char *bt_mac_addr = NULL;

    if (g_bt_is_connect == BT_CONNECT_BLUEZ)
        return DEVICE_FLAG_BLUETOOTH;
    else if(g_bt_is_connect == BT_CONNECT_BSA)
        return DEVICE_FLAG_BLUETOOTH_BSA;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("Open %s failed!\n", path);
        return device_flag;
    }

    ret = read(fd, buff, sizeof(buff));
    if (ret <= 0) {
        printf("Read %s failed!\n", path);
        close(fd);
        return device_flag;
    }

    if (strstr(buff, "hp out"))
        device_flag = DEVICE_FLAG_HEAD_SET;

    close(fd);

    return device_flag;
}

/* Get device name frome device_flag */
const char *get_device_name(int device_flag)
{
    const char *device_name = NULL;

    switch (device_flag) {
        case DEVICE_FLAG_BLUETOOTH:
            device_name = "BLUETOOTH";
            break;
        case DEVICE_FLAG_HEAD_SET:
            device_name = JACK_DEVICE_NAME;
            break;
        case DEVICE_FLAG_LINE_OUT:
            device_name = WRITE_DEVICE_NAME;
            break;
        default:
            break;
    }

    return device_name;
}

void *a2dp_status_listen(void *arg)
{
    int ret = 0;
    char buff[100] = {0};
    struct sockaddr_un clientAddr;
    struct sockaddr_un serverAddr;
    int sockfd;
    socklen_t addr_len;
    char *start = NULL;
    snd_pcm_t* audio_bt_handle;
    char bluealsa_device[256] = {0};
    int retry_cnt = 5;

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("Create socket failed!\n");
        return NULL;
    }

    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, "/tmp/a2dp_master_status");

    system("rm -rf /tmp/a2dp_master_status");
    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        printf("Bind Local addr failed!\n");
        return NULL;
    }

    while(1) {
        addr_len = sizeof(clientAddr);
        memset(buff, 0, sizeof(buff));
        ret = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&clientAddr, &addr_len);
        if (ret <= 0) {
            printf("errno: %s\n", strerror(errno));
            break;
        }
        printf("###### FUCN:%s. Received a malformed message(%s)\n", __func__, buff);

        if (strstr(buff, "status:connect:bsa-source")) {
            if (g_bt_is_connect == BT_DISCONNECT) {
                printf("bsa bluetooth source is connect\n");
                g_bt_is_connect = BT_CONNECT_BSA;
            }
        } else if (strstr(buff, "status:connect")) {
            start = strstr(buff, "address:");
            if (start == NULL) {
                printf("FUCN:%s. Received a malformed message(%s)\n", __func__, buff);
                continue;
            }
            start += strlen("address:");
            if (g_bt_is_connect == BT_DISCONNECT) {
                sleep(2);
                memcpy(g_bt_mac_addr, start, sizeof(g_bt_mac_addr));
                sprintf(bluealsa_device, "%s%s", "bluealsa:HCI=hci0,PROFILE=a2dp,DEV=",
                        g_bt_mac_addr);
                retry_cnt = 5;
                while (retry_cnt--) {
                    ret = snd_pcm_open(&audio_bt_handle, bluealsa_device,
                                       SND_PCM_STREAM_PLAYBACK, 0);
                    if (ret == 0) {
                        snd_pcm_close(audio_bt_handle);
                        g_bt_is_connect = BT_CONNECT_BLUEZ;
                    }
                    usleep(600000); //600ms * 5 = 3s.
                }
            }
        } else if (strstr(buff, "status:disconnect")) {
            g_bt_is_connect = BT_DISCONNECT;
        } else if (strstr(buff, "status:suspend")) {
            g_system_sleep = true;
        } else if (strstr(buff, "status:resume")) {
            g_system_sleep = false;
        } else {
            printf("FUCN:%s. Received a malformed message(%s)\n", __func__, buff);
        }
    }

    close(sockfd);
    return NULL;
}

int main()
{
    int err;
    snd_pcm_t *capture_handle, *write_handle;
    short buffer[READ_FRAME * 2];
    unsigned int sampleRate, channels;
    int mute_frame_thd, mute_frame;
    /* LINE_OUT is the default output device */
    int device_flag, new_flag;
    pthread_t a2dp_status_listen_thread;
    struct rk_wake_lock* wake_lock;
    bool low_power_mode = low_power_mode_check();
    char *silence_data = (char *)calloc(READ_FRAME * 2 * 2, 1);//2ch 16bit
    int socket_fd = -1;

    wake_lock = RK_wake_lock_new("eq_drc_process");

    /* Create a thread to listen for Bluetooth connection status. */
    pthread_create(&a2dp_status_listen_thread, NULL, a2dp_status_listen, NULL);

repeat:
    capture_handle = NULL;
    write_handle = NULL;
    err = 0;
    memset(buffer, 0, sizeof(buffer));
    sampleRate = SAMPLE_RATE;
    channels = CHANNEL;
    mute_frame_thd = (int)MUTE_FRAME_THRESHOD;
    mute_frame = 0;
    /* LINE_OUT is the default output device */
    device_flag = DEVICE_FLAG_LINE_OUT;
    new_flag = DEVICE_FLAG_LINE_OUT;

    printf("\n==========EQ/DRC process release version 1.23===============\n");
    alsa_fake_device_record_open(&capture_handle, channels, sampleRate);
    alsa_fake_device_write_open(&write_handle, channels, sampleRate, device_flag, &socket_fd);
    RK_acquire_wake_lock(wake_lock);

    while (1) {
        err = snd_pcm_readi(capture_handle, buffer , READ_FRAME);
        if (err != READ_FRAME)
            printf("====read frame error = %d===\n",err);

        if (err < 0) {
            if (err == -EPIPE)
                printf( "Overrun occurred: %d\n", err);

            err = snd_pcm_recover(capture_handle, err, 0);
            // Still an error, need to exit.
            if (err < 0) {
                printf( "Error occured while recording: %s\n", snd_strerror(err));
                usleep(200 * 1000);
                if (capture_handle)
                    snd_pcm_close(capture_handle);
                if (write_handle)
                    snd_pcm_close(write_handle);
                goto repeat;
            }
        }

        if (g_system_sleep)
            mute_frame = mute_frame_thd;
        else if(low_power_mode && is_mute_frame(buffer, READ_FRAME))
            mute_frame ++;
        else
            mute_frame = 0;

        if(mute_frame >= mute_frame_thd) {
            //usleep(30*1000);
            /* Reassign to avoid overflow */
            mute_frame = mute_frame_thd;
            if (write_handle) {
                snd_pcm_close(write_handle);
                RK_release_wake_lock(wake_lock);
                printf("%d second no playback,close write handle for you!!!\n ", MUTE_TIME_THRESHOD);
                write_handle = NULL;
            }
            continue;
        }

        new_flag = get_device_flag();
        if (new_flag != device_flag) {
            printf("\nDevice route changed, frome\"%s\" to \"%s\"\n\n",
                   get_device_name(device_flag), get_device_name(new_flag));
            device_flag = new_flag;
            if (write_handle) {
                snd_pcm_close(write_handle);
                write_handle = NULL;
            }
        }

        while (write_handle == NULL && socket_fd < 0) {
            RK_acquire_wake_lock(wake_lock);
            alsa_fake_device_write_open(&write_handle, channels, sampleRate, device_flag, &socket_fd);
            if (write_handle == NULL && socket_fd < 0) {
                printf("Route change failed! Using default audio path.\n");
                device_flag = DEVICE_FLAG_LINE_OUT;
            }

            if (low_power_mode) {
                int i, num = PERIOD_counts / 2;
                printf("feed mute data %d frame\n", num);
                for (i = 0; i < num; i++) {
                    if(write_handle != NULL) {
                        err = snd_pcm_writei(write_handle, silence_data, READ_FRAME);
                        if(err != READ_FRAME)
                            printf("====write frame error = %d, not %d\n", err, READ_FRAME);
                    } else if (socket_fd >= 0) {
                        err = RK_socket_send(socket_fd, silence_data, READ_FRAME * 4); //2ch 16bit
                        if(err != (READ_FRAME * 4))
                            printf("====write frame error = %d, not %d\n", err, READ_FRAME * 4);
                    }
                }
            }
        }

        if(write_handle != NULL) {
            //usleep(30*1000);
            err = snd_pcm_writei(write_handle, buffer, READ_FRAME);
            if(err != READ_FRAME)
                printf("====write frame error = %d===\n",err);

            if (err < 0) {
                if (err == -EPIPE)
                    printf("Underrun occurred from write: %d\n", err);

                err = snd_pcm_recover(write_handle, err, 0);
                if (err < 0) {
                    printf( "Error occured while writing: %s\n", snd_strerror(err));
                    usleep(200 * 1000);

                    if (write_handle) {
                        snd_pcm_close(write_handle);
                        write_handle = NULL;
                    }

                    if (device_flag == DEVICE_FLAG_BLUETOOTH)
                        g_bt_is_connect = BT_DISCONNECT;
                }
            }
        }else if (socket_fd >= 0) {
            if (g_bt_is_connect == BT_CONNECT_BSA) {
                err = RK_socket_send(socket_fd, (char *)buffer, READ_FRAME * 4);
                if (err != READ_FRAME * 4)
                    printf("====write frame error = %d===\n", err);

                if (err < 0) {
                    if (socket_fd >= 0) {
                        printf("socket send err: %d, teardown client socket\n", err);
                        RK_socket_client_teardown(socket_fd);
                        socket_fd = -1;
                    }

                    g_bt_is_connect = BT_DISCONNECT;
                }
            } else {
                if(socket_fd >= 0){
                    printf("bsa bluetooth source disconnect, teardown client socket\n");
                    RK_socket_client_teardown(socket_fd);
                    socket_fd = -1;
                }
            }
        }
    }

error:
    if (capture_handle)
        snd_pcm_close(capture_handle);

    if (write_handle)
        snd_pcm_close(write_handle);

    if (socket_fd >= 0)
        RK_socket_client_teardown(socket_fd);

    pthread_cancel(a2dp_status_listen_thread);
    pthread_join(a2dp_status_listen_thread, NULL);

    return 0;
}
