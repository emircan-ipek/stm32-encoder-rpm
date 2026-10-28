\# STM32 Encoder: Pulse Sayma, Yön ve RPM



STM32F103C8T6 üzerinde A/B kanallı artımlı encoder okuma, yön tespiti ve RPM hesabı (HAL).



\## Özellikler

\- A/B sinyalleri TIM3 donanımsal encoder modunda (TI12, x4) okunur, CPU yükü yoktur

\- Tur başına 100 PPR kabul edilir, x4 modda tur başına 400 sayım

\- TIM4 ile her 1 saniyede hız ve yön güncellenir

\- Pulse, yön ve RPM UART üzerinden gönderilir (115200 8N1)

\- Motor durunca (1 s içinde 0 sayım) RPM = 0, yön = STOP



\## Çıktı formatı

```

PULSE:<toplam> DIR:<CW|CCW|STOP> RPM:<xx.xx>

```



\## Pin ve çevre birimi konfigürasyonu

| Birim | Ayar |

|---|---|

| RCC | HSE 8 MHz, PLL x9, SYSCLK 72 MHz, APB1 36 MHz (timer clock 72 MHz) |

| TIM3 | Encoder Mode TI12, Prescaler 0, Period 65535, giriş filtresi 6, PA6 = A, PA7 = B (dahili pull-up) |

| TIM4 | Prescaler 7199, Period 9999 (1 Hz), update kesmesi, öncelik 1 |

| USART1 | 115200, 8N1, TX = PA9, RX = PA10 |



\## Hesap

RPM = |Δsayım| x 60 / (PPR x 4), ölçüm penceresi 1 s.



| 1 s'deki sayım | RPM |

|---|---|

| 400 | 60.00 |

| 200 | 30.00 |

| 1 | 0.15 |



Yön, 1 s'lik net sayım farkının işaretinden belirlenir (pozitif = CW). Fark int16 ile alındığı için 16-bit sayaç taşmasında da doğru çalışır.



\## Sınırlar

\- 1 s pencerede ±32767 sayım (100 PPR için yaklaşık 4915 RPM). Daha hızlısı için pencere kısaltılıp farklar biriktirilmeli.

\- RPM çözünürlüğü 0.15 RPM.

\- PA6/PA7 5 V toleranslı değildir, encoder 3.3 V ile çalıştırılmalı.

\- 8 MHz HSE kristali varsayılır.



\## Durum

STM32CubeIDE 2.2.0 ile derlendi: 0 hata, 0 uyarı (`docs/build.png`). Donanım üzerinde test edilmedi, hesaplamalar elle doğrulandı.



\## Dosyalar

\- `Core/Src/encoder.c`, `Core/Inc/encoder.h`: encoder modülü

\- `Core/Src/main.c`: saat, UART ve ana döngü

\- `encoder\_rpm.ioc`: CubeMX konfigürasyonu

