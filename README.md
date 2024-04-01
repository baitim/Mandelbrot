
<h1 align="center">Mandelbrot</h1>

## Description

 Implementation of the mandelbrot calculated with no avx and avx instructions, also used "O" flags.

## How to use

1. Clone <br>
    write <code>git clone https://github.com/baitim/Mandelbrot.git</code> in terminal

2. Go to folder <br>
    write <code>cd Mandelbrot</code> in terminal

3. Build <br>
    write <code>make</code> in terminal

4. Run <br>
    write <code>./main</code> in terminal

## Average frame rendering time with standard deviation
<table>
<thead>
<tr>
<th style="text-align: center;"></th>
<th style="text-align: center;"> No optimization </th>
<th style="text-align: center;"> O1 </th>
<th style="text-align: center;"> O2 </th>
<th style="text-align: center;"> O3 </th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Clocks without AVX2, 1e4</td>
<td style="text-align: center;"> 16754 ± 63 </td>
<td style="text-align: center;"> 10477 ± 21 </td>
<td style="text-align: center;"> 10064 ± 24 </td>
<td style="text-align: center;"> 10065 ± 24 </td>
</tr>
<tr>
<td style="text-align: center;">Clocks on AVX2, 1e4</td>
<td style="text-align: center;"> 12696 ± 29 </td>
<td style="text-align: center;"> 3659 ± 13 </td>
<td style="text-align: center;"> 3507 ± 15 </td>
<td style="text-align: center;"> 3509 ± 18 </td>
</tr>
<tr>
<td style="text-align: center;">Acceleration</td>
<td style="text-align: center;"> 1.32 </td>
<td style="text-align: center;"> 2.86 </td>
<td style="text-align: center;"> 2.87 </td>
<td style="text-align: center;"> 2.87 </td>
</tr>
</tbody>
</table>

As a result, we got an acceleration = <b><font size = 4> 4.78 </font></b> if we compare the method without optimizations and with optimization: AVX2 & O3

## Example
<p align="center"><img src="https://github.com/baitim/Mandelbrot/blob/main/images/Mandelbrot.png" width="60%"></p>

<p align="center"><img src="https://github.com/baitim/Mandelbrot/blob/main/images/cat.gif" width="40%"></p>

## Support
**This project is created by [baitim](https://t.me/bai_tim)**

