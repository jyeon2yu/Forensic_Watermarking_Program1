# Forensic Watermarking Program (MFC)
<h3>1. Purpose</h3>
We embed watermark image produced with user’s information in digital image to track illegal distribution path.
We proposed a method that retains JPEG compression robustness by using DWT-DCT and obtain security by embedding watermark using CRT.

<h3>2. Setting for C++, OpenCV</h3>
We maked this program using C++, OpenCV, Visual Studio. <br>
1. Visual Studio
2. C++
3. OpenCV 3.4.16

<h3>3. Architecture</h3>
📌 Host image size must be 512 x 512!

![image](https://user-images.githubusercontent.com/45540117/141438384-2732f750-8cb3-4146-8e11-fc202ed7d031.png)



<h3>4. Program execute</h3>

* Main Dialog

![image](https://user-images.githubusercontent.com/45540117/141437628-9ab45778-f53d-4efe-b0ae-ae1eda7bd1a8.png)


* Login Button
  * Input : User Information
  *  Output : QR Code (Check a QRCODE.jpg in this project directory)

![image](https://user-images.githubusercontent.com/45540117/141439135-e4604ece-fe44-4940-8e94-3d1d9fd0b414.png)

* Image Button
  * Input : 512x512 original image
 
 
* Insert Button
  * Input : OR Code and original image
  * Output : Watermarked images (Check Marked_Image.jpg and Marked_Image.png in this project directory)

![image](https://user-images.githubusercontent.com/45540117/141440634-3e413351-6425-4063-a6b9-8339a38e0837.png)

 
 
* Extract Button
  * Input : Watermarked image
  * Output : QR Code
