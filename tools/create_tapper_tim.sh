#!/bin/bash

./bmp2tim bg512x480_8.bmp bg.TIM 8 -org=512,0 -clut=512,479
./bmp2tim barman.bmp barman.TIM 8 -mpink -org=768,0 -clut=768,257
./bmp2tim tip.bmp tip.TIM 8 -noblack -org=960,128 -clut=768,258
./bmp2tim beer_stats.bmp beer_st.TIM 8 -mpink -org=960,144 -clut=768,259
./bmp2tim beer_sheet.bmp beer_sh.TIM 8 -mpink -org=960,144 -clut=768,259

echo "Done"
echo
echo bg.TIM barman.TIM tip.TIM beer.TIM
echo
exit 0
