#include <string>
#include <iostream>

// Libraries
#include <yaml-cpp/yaml.h>

// Qt
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QKeyEvent>

using namespace std;
using namespace YAML;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr) : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // 2) Load the YAML file
        Node config;
        try
        {
            config = LoadFile("example.yaml");
        }
        catch (const Exception &e)
        {
            cerr << "Error loading example.yaml: " << e.what() << endl;
        }

        if (config["pages"])
        {
            for (const Node &node : config["pages"])
            {
                for (auto element = node.begin(); element != node.end(); ++element)
                {
                    string key = element->first.as<string>();

                    if (key == "label")
                    {
                        string text = element->second["value"]["const"].as<string>();

                        QLabel *label = new QLabel(QString::fromStdString(text), this);
                        layout->addWidget(label);
                    }
                    else if (key == "button")
                    {
                        if (element->second["value"]["const"])
                        {
                            string text = element->second["value"]["const"].as<string>();

                            QPushButton *button = new QPushButton(QString::fromStdString(text), this);
                            layout->addWidget(button);
                        }
                        else if (element->second["value"]["data"])
                        {
                            string ref = element->second["value"]["data"].as<string>();
                            string text = config["data"][ref].as<string>();

                            QPushButton *button = new QPushButton(QString::fromStdString(text), this);
                            layout->addWidget(button);
                        }
                    }
                }
            }
        }

        setLayout(layout);
    }

protected:
    void
    keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Escape)
            QApplication::quit();
        else
            QWidget::keyPressEvent(event);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.showFullScreen();
    return app.exec();
}

// IMPORTANT: Include the MOC-generated file at the end:
#include "main.moc"
