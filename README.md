# EE 4065 - Embedded Digital Image Processing: Homework 3

**Ders:** EE 4065 - Embedded Digital Image Processing  
[cite_start]**Ödev:** Homework 3 (Otsu's Thresholding & Morphological Operations) [cite: 1, 2]  
**Platform:** STM32H7 & Python  

---

### 📷 Orijinal Görüntü (Original Image)
İşlemlerde kullanılan kaynak görüntü.

| **Original Input** |
|:---:|
| <img src="https://github.com/user-attachments/assets/447ebf3d-97d9-4074-9e08-75c7b37d84e2" width="128" height="128" alt="ALF Original"> |
| *ALF (64x64)* |

---

### 1. Otsu Eşikleme (Otsu's Thresholding) [Q1 & Q2]
Gri seviye ve renkli görüntüler üzerinde hesaplanan dinamik eşik değerine göre elde edilen ikili (binary) sonuçlar aşağıdadır. [cite_start]Otsu yöntemi kullanılarak arka plan ve ön plan otomatik olarak ayrıştırılmıştır[cite: 6, 10].

| **Q1: Grayscale Otsu Result** | **Q2: Color Otsu Result** |
|:---:|:---:|
| <img src="https://github.com/user-attachments/assets/cedbfbcc-0a96-40bb-9656-b6fbcc7267b5" width="128" height="128" alt="Q1 Result"> | <img src="https://github.com/user-attachments/assets/589c6991-66a0-4c4c-b299-528b923e7885" width="128" height="128" alt="Q2 Result"> |
| *Gri seviye görüntüden elde edilen maske* | *Renkli görüntüden elde edilen maske* |

---

### 2. Morfolojik Operasyonlar (Morphological Operations) [Q3]
Elde edilen ikili görüntü (binary mask) üzerine uygulanan morfolojik işlemlerin sonuçları. [cite_start]Bu işlemler gürültü giderme ve nesne şekillendirme amacıyla uygulanmıştır[cite: 11, 12, 13].

| **Erosion (Aşındırma)** | **Dilation (Genişleme)** |
|:---:|:---:|
| <img src="https://github.com/user-attachments/assets/315f7d54-d3eb-459b-8202-b43524c38f7f" width="128" height="128" alt="Erosion"> | <img src="https://github.com/user-attachments/assets/f3af2092-eaa8-4464-a8eb-63467fd35c4c" width="128" height="128" alt="Dilation"> |
| *Nesne inceldi / Siyah bölge genişledi* | *Nesne kalınlaştı / Beyaz bölge genişledi* |

| **Opening (Açma)** | **Closing (Kapama)** |
|:---:|:---:|
| <img src="https://github.com/user-attachments/assets/ec7bfdaa-804e-4813-922f-9f9574832243" width="128" height="128" alt="Opening"> | <img src="https://github.com/user-attachments/assets/34621a44-e8cb-42db-b9db-8a5da232f230" width="128" height="128" alt="Closing"> |
| *Gürültü giderme (Erosion -> Dilation)* | *Delik kapatma (Dilation -> Erosion)* |

---
