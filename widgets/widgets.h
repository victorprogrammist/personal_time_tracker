
#ifndef MY_WIDGETS_H
#define MY_WIDGETS_H

#include <QWidget>
#include <QLabel>

#include <functional>

class WidgetSelectObject : public QWidget {
    Q_OBJECT
    QLabel *m_lb_present = nullptr;
    QLabel *m_lb_caption = nullptr;

public:
    WidgetSelectObject(QWidget *parent);

    using TaskRes = std::function<void(uint id)>;
    //using TaskSelect = std::function<void(uint curId, const TaskRes& task)>;
    using TaskSelect = std::function<void(void)>;
    using TaskPresent = std::function<QString(uint id)>;

    void setRequestPresent(const TaskPresent& task);
    void updatePresent();

    TaskSelect reqOpenSelect;

    uint currentId() const { return m_id; }
    void setId(uint id);

    void setCaption(const QString& s) { setText(s); }
    QString caption() const { return text(); }

    QString text() const;
    void setText(const QString& s);

signals:
    void changeId(uint newId);

private:
    uint m_id = 0;
    TaskPresent reqPresent;
};

#endif

