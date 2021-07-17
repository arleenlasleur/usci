ffmpeg -f concat -safe 0 -i man_code.txt -c copy mantest.wav
ffmpeg -i mantest.wav -q 1 mantest.mp3