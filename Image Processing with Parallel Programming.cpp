#include "mpi.h"    
#include <cstdio>   
#include <iostream> 
#include<string.h>
#include <fstream>

using namespace std;

#define KISMI_BOYUT_INFO 61
#define GENISLIK_INFO 59
#define KISMI_DIZI_MASTER_TO_WORKER 25
#define KISMI_DIZI_WORKER_TO_MASTER 26

#define PENCERE 5
int pencere_ortalamasi_al (int * matris, int genislik, int indeks);

int main(int argc,char* argv[])
{
	char processorName[BUFSIZ];
	int  nameLength,
	     mpi_rank,
	     mpi_size;
    
    MPI_Status status;
    

    int * kismi_dizi_girdi;
    int * kismi_dizi_cikti;
    int kismi_boyut;
    int yukseklik;
    int genislik;

	double startTime, 
	       endTime;

    if (argc == 2) {
        // Tam olarak bir tane arguman aldığımızdan emin olduk
        char * girdi_dosya_adresi_orig = new char [128];    // girilen metnin orijimal halini tuttuk
        strcpy(girdi_dosya_adresi_orig, argv[1]);

        char girdi_dosya_adresi[128];                           // girilen metni burada tuttuk (token işlemi yaparken değişecek)
        strcpy(girdi_dosya_adresi, girdi_dosya_adresi_orig);

        char cikti_dosya_adresi[128];
        char * dosya_ismi = NULL;
        if (strstr(girdi_dosya_adresi, "/")){
            // Girilen metinde "/" varmı diye bakıyoruz

            char *p = strtok(girdi_dosya_adresi, "/");
            while (p) {
                if (strstr(p, ".txt")) {
                    // Her slash gördüğümüzde ayırdığımız parçanın içinde ".txt" stringi var mı diye bakıyoruz
                    dosya_ismi = strtok(p, ".");               
                }
                p = strtok(NULL, "/");
            }
        }else if (strstr(girdi_dosya_adresi, ".txt")) {
            // Girilen metinde ".txt" var mı diye bakıyoruz
            dosya_ismi = strtok(girdi_dosya_adresi, ".");
        }

        if (dosya_ismi != NULL) {
            // Eğer kullanıcı geçerli bir dosya ismi verdiyse
            sprintf(cikti_dosya_adresi, "%s_filtered.txt", dosya_ismi); 

            strcpy(girdi_dosya_adresi, girdi_dosya_adresi_orig);
            delete [] girdi_dosya_adresi_orig;

            // Mpi ile ilgili mekanizmalar ilklendirilir.

            /* Initialize the MPI execution environment */
            MPI_Init(&argc,&argv);

            /* Determine the mpi_size of the group */
            MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

            /* Determine the mpi_rank of the calling process */
            MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

            // Debug amaçlı olarak çalışan işlemcilerin isimlerini ve rank'lerini bastır
            /* Get the processor name */
            MPI_Get_processor_name(processorName, &nameLength);
            printf("İslimci ismi: %s, (%d islemciden %d. siradaki)\n",processorName, mpi_size, mpi_rank);

            if (mpi_rank == 0) { // Master
                // Eğer master'daysak
                ifstream girdi_txt_dosyasi; 
                girdi_txt_dosyasi.open(girdi_dosya_adresi);
                if (girdi_txt_dosyasi.is_open())
                {
                    // Verilen dosya adresindeki dosyayı açtık
                    int matris_boyut;

                    // ilk satırdaki yükseklik ve genişlik değerlerini çektik
                    girdi_txt_dosyasi >> yukseklik >> genislik;

                    // Ana giriş ve çıkış dizilerini dinamik olarak aldık
                    matris_boyut = yukseklik * genislik;
                    int * girdi_ana_dizi = new int [matris_boyut];
                    int * cikti_ana_dizi = new int [matris_boyut];

                    // Dosyanın içindeki bütün piksel değerlerini tek tek okuduk ve girdi ve çıktı dizilerine yazdık
                    int piksel;
                    int i = 0, j = 0;
                    while (girdi_txt_dosyasi >> piksel) {
                        girdi_ana_dizi[i] = piksel;
                        cikti_ana_dizi[i] = piksel;
                        i++;
                    }

                    // Kısmi diziler için kısmi boyutları hesapladık
                    kismi_boyut = matris_boyut / mpi_size;
                    
                    // Master'da süre ölçümünü başlattık
                    startTime=MPI_Wtime();

                    // Kısmi boyut ve genişlik değerlerinin worker'lara gönderdik
                    for(int pp=1;pp<mpi_size;pp++)
                        MPI_Send(&kismi_boyut, 1, MPI_INT, pp, KISMI_BOYUT_INFO, MPI_COMM_WORLD);
                    for(int pp=1;pp<mpi_size;pp++)
                        MPI_Send(&genislik, 1, MPI_INT, pp, GENISLIK_INFO, MPI_COMM_WORLD);

                    // Kısmi dizileri (ilk kısmi array hariç) worker'lara gönderdik
                    for(int pp=1;pp<mpi_size;pp++)
                        MPI_Send(&girdi_ana_dizi[pp*kismi_boyut], kismi_boyut, MPI_INT, pp, KISMI_DIZI_MASTER_TO_WORKER, MPI_COMM_WORLD);

                    // Pencere genişliğinin yarısını bul
                    int pf = PENCERE/2; // pencere farkı

                    // İşlenmeye başalanacak ilk piksel ve son piksel indekslerini hesapla (sadece ilk kısmi dizi)
                    int ilk_piksel = (pf * genislik) + pf;      // ikinci satırdaki ikinci piksel
                    int son_piksel = (kismi_boyut - (pf * genislik)) - pf;  // sondan ikinci satırdaki sondan ikinci piksel

                    // işlenecek tüm pikseller için ortalama filtresi hesapla (sadece ilk kısmi dizi)
                    for (int i = ilk_piksel; i < son_piksel; i++) {
                        cikti_ana_dizi[i] = pencere_ortalamasi_al(girdi_ana_dizi, genislik, i);
                    }

                    // ikinci kısmi diziden itibaren worker'lara kısmi dizileri gönder
                    for(int pp=1;pp<mpi_size;pp++)
                    {
                        MPI_Recv(&cikti_ana_dizi[pp*kismi_boyut], kismi_boyut, MPI_INT, pp, KISMI_DIZI_WORKER_TO_MASTER, MPI_COMM_WORLD, &status);	
                    }

                    // Master'da süre ölçümünü durdur
                    endTime=MPI_Wtime();

                    // Çıktı dosyasını aç (yazdırma yapılacak dosya)
                    ofstream cikti_txt_dosyasi; 
                    cikti_txt_dosyasi.open(cikti_dosya_adresi);
                    if (cikti_txt_dosyasi.is_open())
                    {
                        // Yuksekik ve genislik değerlerini yaz
                        cikti_txt_dosyasi << yukseklik << " " << genislik << endl;

                        // Hesaplannan ve cıktı dizisinde birleştirilen değerleri çıktı dosyasına yaz
                        for (i = 0; i < yukseklik * genislik; i++)
                        {
                            if (i>0 && i % genislik == 0) cikti_txt_dosyasi << endl;
                            cikti_txt_dosyasi << cikti_ana_dizi[i] << " ";
                        }

                        // çıktı dosyasını kapat
                        cikti_txt_dosyasi.close();
                    }

                    // girdi dosyasını kapat
                    girdi_txt_dosyasi.close();
                    
                    // Çıktı ve girdi ana_dizilerini sil
                    delete [] girdi_ana_dizi;
                    delete [] cikti_ana_dizi;
                }else {
                    cout << "Verilen adresteki dosya acilamadı." << endl;;
                    return -1;
                }
            }else {
                // Worke^r'ların yapacakları işler

                // Kısmi boyut ve genislik değerlerini master'dan al
                MPI_Recv(&kismi_boyut, 1, MPI_INT, 0, KISMI_BOYUT_INFO, MPI_COMM_WORLD, &status);
                MPI_Recv(&genislik, 1, MPI_INT, 0, GENISLIK_INFO, MPI_COMM_WORLD, &status);

                // Kısmi boyut kadar kısmi dizi oluştur (dinamik)
                kismi_dizi_girdi = new int[kismi_boyut];
                kismi_dizi_cikti = new int[kismi_boyut];

                // Kısmi diziyi master'dan al
                MPI_Recv(kismi_dizi_girdi, kismi_boyut, MPI_INT, 0, KISMI_DIZI_MASTER_TO_WORKER, MPI_COMM_WORLD, &status);

                // İşlenmeye başalanacak ilk piksel ve son piksel indekslerini hesapla (worker'a gelen dizi için)
                int pf = PENCERE/2; // pencere farkı

                int ilk_piksel = (pf * genislik) + pf; // ikinci satırdaki ikinci piksel
                int son_piksel = (kismi_boyut - (pf * genislik)) - pf; // sondan ikinci satırdaki sondan ikinci piksel

                // işlenecek tüm pikseller için ortalama filtresi hesapla (worker'a gelen dizi için)
                for (int i = ilk_piksel; i < son_piksel; i++) {
                    kismi_dizi_cikti[i] = pencere_ortalamasi_al(kismi_dizi_girdi, genislik, i);
                }

                // işlenen diziyi master'a gönder
                MPI_Send(kismi_dizi_cikti, kismi_boyut, MPI_INT, 0, KISMI_DIZI_WORKER_TO_MASTER, MPI_COMM_WORLD);
                
                // dinamik alınan dizileri sil
                delete [] kismi_dizi_girdi;
                delete [] kismi_dizi_cikti;
            }

            // Master'da süre ölçümünü hesapla
            if(mpi_rank==0) /* sadece master gecen sureyi ekrana bassin */
            {
                printf("It takes %f milliseconds to filter the image\n",1000*(endTime-startTime));
            }

            // Mpi işlemlerini sonlandır

            /* Terminate the MPI execution environment */
            MPI_Finalize();
        }else {
            cout << "Verilen adresteki dosya tanımlanamadı.";
            return -1;
        }

    }else {
        cout << "Tam olarak bir komut satiri parametresi girilmelidir." << endl;
        return -1;            
    }

	return 0;
}

int pencere_ortalamasi_al (int * matris, int genislik, int indeks)
{
    int i;
    int j;
    int pf = PENCERE / 2; // pencere farkı
    int toplam = 0;
    int komsu_sayisi = 0;

    int ilk_piksel = (indeks - pf) - (pf*genislik);
    
    for ( i = 0; i < PENCERE; i++)
    {
        int k = ilk_piksel + (i*genislik);
        toplam += matris[k] + matris[k+1] + matris[k+2] + matris[k+3] + matris[k+4];
        komsu_sayisi+=5;
    }

    return toplam / komsu_sayisi;
}