#include <QCoreApplication>
#include <QFile>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTextStream>

class config_label
{
public:
    config_label() {}
    QString text_label;
    QString text_kernel;
    QString text_fdt;
    QString text_append;
};

class config_data
{
public:
    config_data(const QString &file_name);
    QString name;
    QString text_head;
    QString text_default_label;
    QList<config_label *> list_label;
};

config_data::config_data(const QString &file_name)
{
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printf("file open failed: %s", file_name.toLatin1().data());
        return;
    }
    name = file_name;

    QTextStream in(&file);
    bool read_label_flag = false;
    config_label *new_label;
    while (!in.atEnd()) {
        QString line = in.readLine();
        line = line.trimmed();
        if (line.startsWith("LABEL")) {
            read_label_flag = true;
        }

        if (line.startsWith("LABEL") || read_label_flag) {

            if (line.startsWith("LABEL")) {
                new_label = new config_label();
                new_label->text_label = line;
            } else if (line.startsWith("KERNEL")) {
                if (new_label)
                    new_label->text_kernel = line;
            } else if (line.startsWith("FDT")) {
                if (new_label)
                    new_label->text_fdt = line;
            } else if (line.startsWith("APPEND")) {
                if (new_label)
                    new_label->text_append = line;
                list_label.append(new_label);
                new_label = NULL;
            }
        } else if (line.startsWith("DEFAULT") || read_label_flag) {
            text_default_label = line;
        }
        else {
            text_head += line + "\n";
        }
    }
    file.close();
}


void config_label_show(const QString &file_name)
{
    config_data *config = new config_data(file_name);

    QString default_label = config->text_default_label.section(' ', 1);
    for (int i = 0; i != config->list_label.size(); ++i )
    {
        config_label *label_new = config->list_label.at(i);

        if (default_label == label_new->text_label.section(' ', 1))
            printf("%s\n", QString("* %1.%2").arg(i).arg(label_new->text_label.section(' ', 1)).toLatin1().data());
        else
            printf("%s\n", QString("  %1.%2").arg(i).arg(label_new->text_label.section(' ', 1)).toLatin1().data());
    }
}

void config_label_set(const QString &file_name, unsigned int index)
{
    config_data *config = new config_data(file_name);
    QString new_default_label;

    if (index < (unsigned int)config->list_label.size()) {
        config_label *label_new = config->list_label.at(index);
        new_default_label = label_new->text_label.section(' ', 1);
    } else {
        printf("index is invaild\n");
        return;
    }

    QFile file(file_name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    in << config->text_head;
    if (new_default_label.isEmpty())
        in << config->text_default_label << endl;
    else
        in << "DEFAULT " << new_default_label << endl;

    for (int i=0; i!=config->list_label.size(); ++i )
    {
        config_label *label_new = config->list_label.at(i);
        in << label_new->text_label << endl;
        in << "\t" << label_new->text_kernel << endl;
        in << "\t" << label_new->text_fdt << endl;
        in << "\t" << label_new->text_append << endl;
    }
    file.close();
}

int main(int argc, char *argv[])
{
    QString file_name = "/boot/mmc1_panguboard_extlinux/extlinux.conf";
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption opt_set(
                "s",
                QCoreApplication::translate("main", "test"),
                QCoreApplication::translate("main", "test"));
    parser.addOption(opt_set);

    QCommandLineOption opt_show(
                "l",
                QCoreApplication::translate("main", "test"));

    parser.addOption(opt_show);
    parser.process(a);

    bool show = parser.isSet(opt_show);
    if (show) {
        config_label_show(file_name);
        return 0;
    }

    QString index = parser.value(opt_set);
    config_label_set(file_name, index.toUInt());
    config_label_show(file_name);

    //return a.exec();
}


