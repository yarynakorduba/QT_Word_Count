#include <QString>
#include <QFile>
#include <iostream>
#include <QTextStream>
#include <QMap>
#include <cmath>
#include <iterator>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>
#include "timing_v1.hpp"

using namespace std;


QMap<QString, int> words;
QString output;
QMap<QString, int>::iterator it;
QMutex mutex;
QWaitCondition bufferNotEmpty;


QStringList reading(const QString& filename) {
    QStringList lst;
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
          QString l = in.readAll().toLower();
          for (QChar el : l) {
              if (el.isPunct()) {l.remove(el);}
          }

          lst = l.simplified().split(' ', QString::SkipEmptyParts);

       inputFile.close();

    }
    return lst;
}

QList<int> lst_division(QStringList& data_lst, int threads) {
    QList<int> general;
    int pointer = 0;
    int division = std::ceil((float)data_lst.size()/threads);
        while (pointer+division < data_lst.size()) {

            general.append(pointer);
            general.append(pointer+division);

            pointer += division + 1;
        }

        if (pointer != data_lst.size()-1) {
            general.append(pointer);
            general.append(data_lst.size()-1);
        }


    return general;
}


class CountingThread : public QThread {

    public:
        CountingThread(const QStringList& data_lst, int num_start_i, int num_fin_i);
        void run();

    protected:
        const QStringList& data;
        const int num_start; // Нагадайте -- анекдот розкажу, про "справжній посередині, повертаю"
        const int num_fin;   // Крім того, start -- то такий метод QThread, а Ви його...



 };


CountingThread::CountingThread(const QStringList& data_lst,\
                               int num_start_i, int num_fin_i):
    data (data_lst), num_start (num_start_i), num_fin(num_fin_i)
{
}


void CountingThread::run() {
    for (int a=num_start; a<=num_fin; a++) {
        mutex.lock();
        ++words[data[a]];
        mutex.unlock();
    }

}

int main(int argc, char *argv[])
{
   // ----------------------------------------------
   // Зробіть тут читання із файлу конфігурації, а краще -- з командного рядка
   int num_threads = 2;

   QString base_path   {"../QT_Word_Count-/"};
   QString out_filename{base_path + QString("result_%1.txt").
                                        arg(num_threads, 2, 10, QChar('0'))};
   QString in_filename {base_path + "fl.txt"};
   // ----------------------------------------------

   QStringList words_lst = reading(in_filename);
   if (words_lst.isEmpty()) {
       cerr << "No data in the file"<< endl;
       return -1;
   }
   QList<int> num_lst = lst_division(words_lst, num_threads);

   cout << "PROGRAM DESCRIPTION" << endl;
   cout << "TOTAL QUANTITY OF WORDS: " << words_lst.size() << endl;
   cout << "Threads: " << num_threads << endl;
   cout << "Parts: ";
   for(int i = 0; i< num_lst.size(); ++i)
   {
       cout << num_lst[i] << ", ";
   }
   cout << endl;

   auto creating_threads_start_time = get_current_time_fenced();

   QList<CountingThread*> thread_lst;
   int num_pointer = 0;
   for (int el=0; el<num_threads; el++) {
     //  cout <<num_lst[num_pointer] << "AND" << num_lst[num_pointer+1]<< " ";
               thread_lst.append(new CountingThread(\
                                     words_lst, num_lst[num_pointer], num_lst[num_pointer+1]));
               num_pointer += 2;
   }


   //---------------------------------------------------------------
   // Це все ще не дуже хороший спосіб виміру! Потім мусимо переключитися
   // на Performance counters, але це буде потім, а так -- краще, ніж QTimer.
   // Performance counters -- див. PAPI тут:
   // Архітектура комп'ютерних систем (CS.02.17) --> Практична 2. Розпаралелення задач із явним використанням потоків ОС --> Вимірювання часу
   // (Пряме посилання не даю, з міркувань безпеки).
   auto indexing_start_time = get_current_time_fenced();

   for (auto thread: thread_lst) {
       if(num_threads>1)
       {
           thread->start(); // thread->run(); STARTS CODE IN THIS THREAD! Use start to run code in other thread.
       }else{
           thread->run(); // Do not use threads at all.
       }



   }

   for (auto thread: thread_lst){

        thread->wait();
   }

   auto indexing_done_time = get_current_time_fenced();

   //---------------------------------------------------------------
   for(auto x: thread_lst)
       delete x;

   auto time_res = to_us(indexing_done_time - indexing_start_time);
   auto creating_threads_time = to_us(indexing_start_time - creating_threads_start_time);
   cout << "INDEXING TIME: " << time_res << " us " << endl;
   cout << "THREADS CREATING TIME: " << creating_threads_time << " us " << endl;
   QFile output_file(base_path+out_filename);
   if (!output_file.open(QIODevice::WriteOnly)) {
       cerr << "Coulnt write ti file with result" << endl;
       return -1;
   }

   QTextStream output_stream(&output_file);
   int total_words = 0;
   for (auto it = words.begin(); it != words.end(); ++it) {
       // Format output here.
       output += QString("%1 : %2").arg(it.key()).arg(it.value()) + " |, ";
       total_words += it.value();
   }

 output_stream << "Total words: " << total_words << endl;
 output_stream << "Total time: " << time_res << endl;
 output_stream << output;
 output_file.close();

}

