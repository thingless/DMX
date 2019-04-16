#!/usr/bin/env python
from __future__ import print_function
import sys
import pyaudio
import numpy as np
import btrack
import time
import argparse
import threading

#if ya forget what all numbers mean: https://stackoverflow.com/questions/35970282/what-are-chunks-samples-and-frames-when-using-pyaudio
MIC_RATE=44100
SECONDS_TO_SAMPLE=5

bpm_lock=threading.Lock()
bpm=120
beat_onset_fn=lambda:True

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def _steam_callback(in_data, frame_count, time_info, status):
    data = np.fromstring(in_data, dtype=np.float32)
    #onset_fn=btrack.calculateOnsetDF(data)
    beats = btrack.trackBeats(data)
    b = 60.0/round(np.mean(np.diff(beats)),2)
    eprint('Detected bpm', b)
    global bpm
    global beat_onset_fn
    with bpm_lock:
        bpm = b
        #beat_onset_fn = onset_fn
    return (None, pyaudio.paContinue)

def start_calc_beat_delta():
    audio = pyaudio.PyAudio()
    audio.open(format=pyaudio.paFloat32, channels=1, rate=MIC_RATE, input=True, output=False, frames_per_buffer=MIC_RATE*SECONDS_TO_SAMPLE, stream_callback=_steam_callback)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Detects BPM and updates based on it")

    parser.add_argument("--brightness", type=int, default=20, help="Maximum brightness per channel (0-255)")
    parser.add_argument("--leds", type=int, default=4, help="How many LEDs to generate channels for? (def 4)")
    args = parser.parse_args()
    start_calc_beat_delta()

    while True:
        with bpm_lock:
            beat_onset_fn()
            bps=bpm/60.0
        print(3, 0, 5,5,5,5,5,5,5,5,5,5,5,5)
        print(int(round((100.0/bps)-3)), 0, 0,0,0,0,0,0,0,0,0,0,0,0)
        print('END')
        sys.stdout.flush()
        time.sleep(SECONDS_TO_SAMPLE-0.25) #nothing going to happen for a while
