
#include "widgets.h"

#include <QPushButton>
#include <QBoxLayout>

WidgetSelectObject::WidgetSelectObject(QWidget *parent)
        : QWidget(parent) {

    auto *bt_sel = new QPushButton("...");
    bt_sel->setFixedWidth(30);

    auto *bt_clear = new QPushButton("x");
    bt_clear->setFixedWidth(30);

    m_lb_caption = new QLabel(this);
    m_lb_caption->setVisible(false);

    m_lb_caption->setSizePolicy(
        QSizePolicy::Minimum,
        QSizePolicy::Fixed);

    m_lb_present = new QLabel(this);
    m_lb_present->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed);

    auto* lt = new QHBoxLayout;
    lt->setContentsMargins(0,0,0,0);

    setLayout(lt);

    lt->addWidget(m_lb_caption);
    lt->addWidget(bt_sel);
    lt->addWidget(bt_clear);
    lt->addWidget(m_lb_present);

    updatePresent();

    connect(bt_clear, &QPushButton::clicked, [this] { setId(0); });

    connect(bt_sel, &QPushButton::clicked, [this] {
        if (reqOpenSelect)
            reqOpenSelect();
    });
}

QString WidgetSelectObject::text() const {
    return m_lb_caption->text();
}

void WidgetSelectObject::setText(const QString& s) {
    m_lb_caption->setVisible(!s.isEmpty());
    m_lb_caption->setText(s);
}

void WidgetSelectObject::setId(uint id) {
    if (m_id == id) return;
    m_id = id;
    updatePresent();
    emit changeId(id);
}

void WidgetSelectObject::setRequestPresent(const TaskPresent& task) {
    reqPresent = task;
    updatePresent();
}

void WidgetSelectObject::updatePresent() {
    QString pres;

    if (!reqPresent)
        pres = QString("<id:%1>").arg(m_id);
    else
        pres = reqPresent(m_id);

    m_lb_present->setText(pres);
}
