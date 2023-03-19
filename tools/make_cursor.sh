#!/bin/bash

./bmp2tim cursor.bmp cursor.TIM 8 -mpink -org=1012,487 -clut=768,511
wine bin2h.exe cursor.TIM cursor.h

exit 0
