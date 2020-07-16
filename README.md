# Image-Processing-with-Parallel-Programming
Reduce noise on the image thanks to the Average Filter used in image processing by parallel programming.

Paralel programlama yaparak görüntü işlemede kullanılan Ortalama Filtre sayesinde görüntü üzerindeki gürültüyü azaltmak.

Görüntü işleme algoritmaları bir görüntüyü bilgisayar yardımıyla işleyebilmek amacıyla ilk etapda görüntüyü iki veya daha fazla boyuta sahip matrise dönüştürür. Bu matrisin her bir elemanının değeri ilgili görüntünün her bir pikselinin sayısal değerine eşittir.

**Ortalama Filtre** belirli bir pencere aralığında (projede pencere boyutu 5x5 olarak kullanılıyor) sayıların ortlamasını bulmayı ve bu sayede söz konusu sayılar arasındaki aşırı farklılıkları ortadan kaldırmayı hedeflenmektedir. Yani Ortalama filtre sayesinde komşularından belirgin bir şekilde ayrışan piksellerin tespit edilmesi ve yumuşatılması amaçlanmaktadır.

Projede değerlendirmeniz amacıyla **görüntü dosyası(Goruntu3.bmp)** ve bu dosyanın **matris temsilleri (GoruntuMatrisi3.txt)** içeren düz yazı dosya paylaşılmıştır. Matris dosyasının ilk satırında ilgili matrisin kaç satırdan ve kaç sütundan oluştuğu bilgisi yer almaktadır. Görüntüye tuz ve biber gürültüsü **(salt&pepper noise)** eklenmiştir. Ortalama filtre sayesinde bu gürültünün görüntüler üzerinden kaldırılması hedeflenmektedir. Bu doğrultuda matris dosyasından ortalama filtre uygulayarak yeni bir matris oluşturulması ve bu matris görüntüyü dönüştürerek paylaşılan orjinal görüntüyle karşılaştırlması yapılır.

Ortalama filtre bir görüntüdeki her bir piksel için, pencere boyutundaki komşu piksellerin ortalama değerini elde eder ve ilgili piksele bu ortalama değeri atar. Bu işlem her piksel için sıra ile uygulanır. 5x5'lik komşuluk beklenirken, filtre uygulanan piksel (ilgili pencerenin 3.satırında ve 3. sütununda yer alan piksel) bu matirisin ortasına getirilerek komşuları belirlenir.

- 5x5'lik ortalama filtrede ilgili pikselin 2'şer komşuluklarına bakılır. Örneğin (x,y) pozisyonundaki bir pikselin komşuları aşağıdaki gibidir.

![5x5_matris](https://user-images.githubusercontent.com/66306220/87730193-c249de00-c7cf-11ea-9e83-2ccb5d481610.jpg)

- 5x5’lik ortalama filtrede matrisin *ilk 2 satırındaki* ve *ilk 2 sütunundaki*, ayrıca *son 2 satırındaki* ve *2 sütunundaki piksellerine* filtre ~~uygulanmaz.~~ Örneğin 5x5’lik ortalama filtre için, GoruntuMatrisi3.txt isimli dosya içerisindeki filtre uygalanacak ilk değer 114’dür.

![Uygalacak_ilk_filtre](https://user-images.githubusercontent.com/66306220/86361999-5159e080-bc7d-11ea-888e-689484bd3951.png)

## Değeri 114 olan piksel için filtrelenin nasıl olacağı aşağıda açıklanmıştır.

**1.**	114’ ün 5x5’lik komşu değerleri tespit edilir. (kırmızı dikdörtgenin içerisinde kalan kısım)

**77-113-195-77-76-82-82-77-87-21-105-106-114-105-101-5-128-122-122-122-72-82-82-44-71**

**2.**	Bu değerler toplanır: **2268**

**3.**  Ortalama hesaplanır: **2268 / 25 = 90**

Filtre uygalancak pikselin eski değeri **114**, yeni değeri **90**’dır. Dolayısıyla sonuç matrisinde **114**’ün bulunduğu konuma **90** değeri kaydedilecektir. (Ana matris üzerinde değişiklik yapılmıyor.)

Bu projenin seri programlama yaparak kodlanan versiyonuna buradan ulaşabilirsiniz. [imageprosesing](https://github.com/halildurmuss/imageprosesing)

# Parallel Programming

Bu projede [imageprosesing](https://github.com/halildurmuss/imageprosesing)
 projesinde seri olarak kodladığım görüntü işleme algoritmasını MPI kütüphanesinin noktadan noktaya haberleşme rutinlerini kullanarak paralel programlama yapılmıştır. Süre ölçümünde filtreleme işleminin yanı sıra, paralel programlamanın doğasından kaynaklanan ekstra işler de (verinin işlemcilere paylaştırılması ve ara sonuçların toplanması sonrasında filtrelenmiş matrisin elde edilmesi) göz önüne alınmıştır.
 
 Giriş matrisinin dosyadan okunmasını ve sonrasında ilgili veri yapısına kaydedilmesi, verinin ilgili kısımlarının diğer bilgisayarlara paylaştırılması, işlenen kısmi verilerin bilgisayardan toplanması ve sonuç matrisinin oluşturulması işlemlerinden sadece bir bilgisayarın (master) sorumlu olduğu unutulmaması gerekir.
 
 ![time](https://user-images.githubusercontent.com/66306220/87730699-3d5fc400-c7d1-11ea-90e1-2ba759c46b44.png)

![speedup](https://user-images.githubusercontent.com/66306220/87730727-4cdf0d00-c7d1-11ea-801c-c095eb131445.png)

Kümeyi oluşturan sunucuların tek bir sistem gibi çalışması için birbirleriyle çok hızlı haberleşmesine sağlayacak bir ağ alt yapısına sahip olması gerekir. Beowolf kümeleri sıradan bilgisayar ile oluşturulabilir. Benim projemde kullandığım sistem yeterli değil çünkü tabloda çıkan sonuca baktığımızda speedup değerleri 1’den küçük değerler. Sistem yeterli olsaydı Speedup değerlerini 1’den büyük olacaktı ve projemi başarı ile paralelleştirmiş olacaktım.

![speedup](https://user-images.githubusercontent.com/66306220/87730938-dd1d5200-c7d1-11ea-95cf-55c64b430e7e.png)

![Grafik](https://user-images.githubusercontent.com/66306220/87730973-f1f9e580-c7d1-11ea-9b74-614ffa08e586.png)

- GoruntuMatrisi3.txt giriş dosyasının  ilk 10 satırını ve 10 sütünunu içeren bölge çıktısı:

![matrix](https://user-images.githubusercontent.com/66306220/87731084-43a27000-c7d2-11ea-8193-37905ff3ab3e.png)

- GoruntuMatrisi3.txt giriş dosyasının **filtrelenmiş** değerlerini içeren çıkış dosyasının ilk 10 satırını ve 10 sütununu içeren bölgenin çıktısı

![matrix1](https://user-images.githubusercontent.com/66306220/87731109-52892280-c7d2-11ea-80e0-e3f966895c6c.png)
## Proje Çalıştırma

Projeyi **mpicxx** ile derleyip, **mpirun** ile çalışıp çalışmadığına sanal olarak test edilebilir. En sonunda **mpiexec** komutu ile küme üzerinde koşturarak süre ölçümünü öğrenilebilir.
Süre ölçümünde **MPI_Wtime** fonksiyonu kullanılmıştır. Ölçümü ve ekrana yazdırma işlemini sadece master bilgisayar üzerinden yapıyor.

## Örnek Çıktı

### İlk hali: Goruntu3.jpg

![Goruntu3](https://user-images.githubusercontent.com/66306220/87731300-d5aa7880-c7d2-11ea-9eb9-4cf60911eace.jpg)

### Filtre uygulanmış hali: GoruntuMatrisi3_filtered.jpg

![GoruntuMatrisi3_filtered](https://user-images.githubusercontent.com/66306220/87731439-2c17b700-c7d3-11ea-91f9-c93dfb76f5a3.jpg)



