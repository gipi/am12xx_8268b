#ifndef AUDIO_LINEIN_H
#define AUDIO_LINEIN_H

#include "audio_record.h"


void *AudioLineInStart(linein_input_t *input);
int AudioLineInStop(void *handle);
void AudioLineInSetDelay(void *handle,unsigned int delay);
unsigned int AudioLineInGetDelay(void *handle);
void AudioLineInIncDelay(void *handle);
void AudioLineInDecDelay(void *handle);

#endif

