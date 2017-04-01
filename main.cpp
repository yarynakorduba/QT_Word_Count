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

QList<QStringList> lst_division(QStringList& data_lst, int threads) {
    QList<QStringList> general;
    int pointer = 0;
    int temp = 0;
    int division = std::ceil((float)data_lst.size()/threads);
    while (pointer < data_lst.size()) {
        QStringList tmp = {};
        for (int el=pointer; el < pointer+division; el++) {
            if (el < data_lst.size()) {
            tmp.append(data_lst[el]);

        }
        }

        general.append(tmp);
        pointer += division;
    }

    return general;
}



QMap<QString, int> words;
QString output;
QMap<QString, int>::iterator it;
QMutex mutex;
QWaitCondition bufferNotEmpty;
QTime time_result;

class CountingThread : public QThread {

    public:
        CountingThread(const QStringList& data_lst);
        void run();

    protected:
        const QStringList& data;


 };


CountingThread::CountingThread(const QStringList& data_lst): data (data_lst) {
}


void CountingThread::run() {
    for (int a=0; a<data.size(); a++) {
        mutex.lock();
            ++words[data[a]];
        mutex.unlock();
    }

}

int main(int argc, char *argv[])
{
   QString base_path = {"/home/yaryna/AKS_main/"};
   QStringList words_lst = reading(base_path + "fl.txt");
   //cout << words_lst.size();

   if (words_lst.isEmpty()) {
       cerr << "No data in the file"<< endl;
       return -1;
   }

   cout << "PROGRAM DESCRIPTION" << endl;
   cout << "TOTAL QUANTITY OF WORDS: " << words_lst.size() << endl;
   QList<QStringList> lst = lst_division(words_lst, 3);
   QList<CountingThread*> thread_lst;
   for (int el=0; el<lst.size(); el++) {
               thread_lst.append(new CountingThread(lst[el]));
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

