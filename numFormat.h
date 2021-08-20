
#ifndef TimeFormatDelegate_H
#define TimeFormatDelegate_H

#include <QStyledItemDelegate>

class TimeFormatDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QString displayText(const QVariant &value, const QLocale &) const override {

        uint tmSecs = value.toUInt();

        if (tmSecs < 60)
            return QString::number(tmSecs) + " secs";

        if (tmSecs < 3600)
            return QString::number(tmSecs/60) + " mins";

        return QString::number( tmSecs/3600., 'f', 2 ) + " hrs";

        //return locale.toString(value.toDouble(), 'f', 2);
    }

signals:

public slots:

};

#endif // TimeFormatDelegate_H
