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

using namespace std;


QMap<QString, int> words;
QString output;
QMap<QString, int>::iterator it;
QMutex mutex;
QWaitCondition bufferNotEmpty;
QTime time_result;


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
        CountingThread(const QStringList& data_lst, const int& num_start, const int& num_fin);
        void run();

    protected:
        const QStringList& data;
        const int& start;
        const int& finish;



 };


CountingThread::CountingThread(const QStringList& data_lst,\
                               const int& num_start, const int& num_fin):
    data (data_lst), start (num_start), finish(num_fin){
}


void CountingThread::run() {
    for (int a=start; a<=finish; a++) {
        mutex.lock();
            ++words[data[a]];
        mutex.unlock();
    }

}

int main(int argc, char *argv[])
{
   QString base_path = {"/home/yaryna/AKS_main/"};
   QStringList words_lst = reading(base_path + "fl.txt");

   if (words_lst.isEmpty()) {
       cerr << "No data in the file"<< endl;
       return -1;
   }

   cout << "PROGRAM DESCRIPTION" << endl;
   cout << "TOTAL QUANTITY OF WORDS: " << words_lst.size() << endl;
   int num_threads = 5;
   QList<int> num_lst = lst_division(words_lst, num_threads);
   QList<CountingThread*> thread_lst;
   int num_pointer = 0;
   for (int el=0; el<num_threads; el++) {
     //  cout <<num_lst[num_pointer] << "AND" << num_lst[num_pointer+1]<< " ";
               thread_lst.append(new CountingThread(\
                                     words_lst, num_lst[num_pointer], num_lst[num_pointer+1]));
               num_pointer += 2;
   }




   time_result.start();

   for (auto thread: thread_lst) {

       thread->run();
   }

  for (auto thread: thread_lst){

   thread->wait();}



   //---------------------------------------------------------------
   for(auto x: thread_lst)
       delete x;

   int time_res = time_result.elapsed();
   cout << "TOTAL TIME: " << time_res << " ms " << endl;
   QFile output_file(base_path+"result.txt");
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

