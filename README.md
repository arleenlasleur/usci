# PC to Arduino ultrasonic command interface
  
**This project is using these Arduino libraries:**  
https://gist.github.com/WinstonArrocena/50f5c561868717effdc0 (TM1637.cpp)  
https://gist.github.com/WinstonArrocena/0e4e4f8976ed482544eb (TM1637.h)  
https://github.com/kosme/arduinoFFT  
  
This project is attempt to remote control some Arduino-driven device via PC, without interfacing through USB (with necessary drivers), Eth LAN or WLAN. The only adapter device is audiomixer, compound of some resistors.
  
## Controls: (in usci\usci.ino file)
  
#define SAMPLES     (64)  // 2^n    - FFT size  
#define HFVOL_THR   (65)  // 0..127 - volume threshold of carrier signal  
  
## Debugging/tuning:
  
Uncomment Serial output and enable Arduino IDE plotter mode. Threshold must meet both absence of unwanted responses as well as stable high level while carrier tone is sound. Digitized input is shown by D13 LED. False positive input states is allowed while input cable is unplugged.
  
## Schematic:
  
Shown in sch_mixer.svg
  
## Requirements:
  
- working audio output on any OS (Windows, Debian, DOS etc)
- ability to emit ultrasound (above 18 kHz). This may be overridden by altering input files for low-end audiocards.   To prevent audio clicks, fronts and falls of carrier tone could be faded in/out. The files shipped come in PCM16LE-44100 wave, tone recorded is 18500 Hz.
- reasonable jitter, not ruining mark signals front/back porch. If your audio doesn't allow signal to pass through, try generating more long mark/space files. Longer chunks means more readability as well as lower speed. That's the effect of manchester code (it requires high stability of T and 2T periods).
  
## Tools used:
  
- PHP CLI (tested on PHP 5.2-x86 for Windows)
- ffmpeg
  
## Code generation:
  
1. Modify $payload variable in man_conv.php
2. Execute:
   php man_conv.php
   This will write new text to man_code.txt
3. Execute:
   domanch.bat
   This will concat pieces of mark/space/double mark/double space into one single code wave, then convert it in mp3.
  
If you want singlequery HTML frontend, you can reencode mp3 files to base64 and paste it in HTML <audio> src= attr, but make sure it's short enough to fit in 64Kbyte since data-URI of HTML won't accept URI bigger than 64Kb.
  
## Example HTML:

                                        |------------- 64 Kb max  --------------|
    <audio controls>                   \/                                       |
    <source src="data:audio/mpeg;base64,SUQzBAAAAAAAI1RTU0UAAAAPAAADTGF2ZjU3    |
    //////more//base64//data//here////////VVVVVVVVVVVVVVVVVVVVVVVVVVU=" />      |
    </audio>                                                         ^-----------
  
## Protocol:
  
First 1111 is carrier hijack (manchester decode process training T and 2T timings during it), then 0100 preamble. All further data is considered correct payload. No flow control, error control or checksums is applied. Implement your own if necessary (CRC = firstbyte XOR secondbyte XOR thirdbyte etc, for example).
  
## ATX-flavor onboard audio connector (case front panel connector):
  
    pin  HDA:           AC97:          post-2018 HDA:  +--------+
                                                       | 2   10 |
     1   Mic2_l         Mic            Mic2_L          | :::.:  |
     2   Gnd            Gnd            Gnd             | 1   9  |
     3   Mic2_R         Mic power      Mic2_R          +--------+
     4   -Acz_det       NC             -Acz_det
     5   Line2_R        Line out(r)    Line2_R     MB output logic:
     6   Gnd            NC             Sen1_Ret
     7   FAudio_JD      NC             Sens_Send   Microphonic devices may record from Mic2 when Sens_Send-to-Sen1_Ret
     8   -----          -----          -----        circuit closed, do not record when open
     9   Line2_L        Line out(l)    Line2_L     Speaking devices may play to Line2 when Sens_Send-to-Sen2_Ret
     10  Gnd            NC             Sen2_Ret     circuit closed, do not play when open
  
__NOTE: this info is mostly useless, many mainboards observe output logic, so for desktop applications you need to use stereo doubler connector (1 jack to 2 sockets).__
  
Credits:
  
shajeebtm: Arduino 32-band audio spectrum analyzer
victornpb: manchester decoder/js encoder
  
  
# Arduino ультразвуковой командный интерфейс для ПК
  
**Используемые Arduino библиотеки:**  
https://gist.github.com/WinstonArrocena/50f5c561868717effdc0 (TM1637.cpp)  
https://gist.github.com/WinstonArrocena/0e4e4f8976ed482544eb (TM1637.h)  
https://github.com/kosme/arduinoFFT  
  
Этот проект демонстрирует управление в обход любых способов, требующих специального драйвера/оборудования. Переходник представляет собой обычный смеситель аудио на резисторах.
  
## Параметры: (в файле usci\usci.ino)
  
#define SAMPLES     (64)  // 2^n    - ширина окна БПФ  
#define HFVOL_THR   (65)  // 0..127 - порог громкости несущего сигнала  
  
## Отладка/настройка:
  
Раскомментируйте строки вывода в Serial и включите режим отрисовки графика в Arduino IDE. Порог должен быть достаточно высоким чтобы не наблюдалось ложных сработок, но достаточно низким чтобы на моментах звучания несущего тона наблюдать чёткую единицу. Контролировать оцифрованный уровень можно по состоянию светодиода на выходе D13. Ложные срабатывания висящего в воздухе входа допустимы.
  
## Принципиальная схема:
  
Показано в sch_mixer.svg
  
## Требования к системе:
  
- работающий аудиодрайвер, на любой ОС
- способность звуковой карты воспроизводить в УЗ полосе частот (выше 18 кГц). Это ограничение можно преодолеть, изготовив новые звуки для аудиокарт низшего класса. Во избежание щелчков на записи, фронты/спады несущего сигнала можно подвергнуть фильтру fadein/fadeout. Приложенные файлы являются целочисленным PCM 16 бит с дискретизацией 44100 (стандартный WAV файл Microsoft CD-качества), записанный тон 18500 Гц.
- вменяемый аудиотракт, джиттер которого из-за переходных процессов не размажет фронты/спады сигнала. Если звук не может пройти в ваших условиях, попробуйте создать звуки с более длинными/низкочастотными тонами/паузами. Более длинные куски более читаемы, но медленнее воспроизводятся. Этот эффект накладывает код Manchester, требовательный к соблюдению периодов бита и полубита.
  
## Инструменты:
  
- команднострочный PHP (тестировалось с PHP 5.2-x86 для Windows)
- ffmpeg
  
## Изготовление кодов:
  
1. Отредактируйте переменную $payload в файле man_conv.php
2. Запустите:
   php man_conv.php
   Сценарий запишет новый текст кодов в файле man_code.txt
3. Запустите:
   domanch.bat
   Эти команды склеят отдельные звуки тонов/пробелов в единый WAV файл, затем конвертируемый в mp3.
  
Если вам нужен HTML без включений (в один запрос), можно перевести mp3 звуки в base64 а затем вставить в атрибут src= HTML-тегов <audio> в виде data-URI строк, но они должны умещаться в 64 килобайта (таковые не могут превышать 64 Кб)
  
## Образец HTML:
  
                                        |------------- 64 Kb max  --------------|
    <audio controls>                   \/                                       |
    <source src="data:audio/mpeg;base64,SUQzBAAAAAAAI1RTU0UAAAAPAAADTGF2ZjU3    |
    //////more//base64//data//here////////VVVVVVVVVVVVVVVVVVVVVVVVVVU=" />      |
    </audio>                                                         ^-----------
  
## Протокол:
  
Первые 1111 - захват несущей, в это время процедура декодера выясняет длительности T и 2T, затем преамбула 0100. Все принятые после неё данные считаются правильными и записываются в полезную нагрузку. Никакого управления потоком, коррекции ошибок, контрольных сумм не применяется. Вы можете реализовать собственные если нужно, например XORить байты по очереди (CRC = байт1 XOR байт2 XOR байт3 и т д).
  
## Разъём аудио ATX-подобных системных плат (он же разъём аудио для передней панели):
  
    pin  HDA:           AC97:          post-2018 HDA:  +--------+
                                                       | 2   10 |
     1   Mic2_l         Mic            Mic2_L          | :::.:  |
     2   Gnd            Gnd            Gnd             | 1   9  |
     3   Mic2_R         Mic power      Mic2_R          +--------+
     4   -Acz_det       NC             -Acz_det
     5   Line2_R        Line out(r)    Line2_R     Логика вывода платы:
     6   Gnd            NC             Sen1_Ret
     7   FAudio_JD      NC             Sens_Send   Звук с устройств на линии Mic2 поступает в систему, если цепь
     8   -----          -----          -----        Sens_Send-Sen1_Ret замкнута, не участвуют если разомкнута
     9   Line2_L        Line out(l)    Line2_L     Звук выводится из системы в устройства на линии Line2, если цепь
     10  Gnd            NC             Sen2_Ret     Sens_Send-Sen2_Ret замкнута, не выводится если разомкнута
  
Этот разъём обычно бесполезен для настольных приложений, т. к. большинство плат соблюдают логику коммутации, т. е. невозможно получить звук на переднем и заднем выходах одновременно (для этих случаев, имеет смысл использовать стерео раздвоитель).
  
## Исходники:
  
shajeebtm: Arduino 32-полосный анализатор спектра на аудиодиапазон
victornpb: кодек manchester на js/wiring
