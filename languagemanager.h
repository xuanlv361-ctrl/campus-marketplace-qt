#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QString>

class QComboBox;
class QWidget;
class QEvent;

class LanguageManager : public QObject
{
    Q_OBJECT

public:
    enum class Language {
        Chinese,
        English
    };

    static LanguageManager &instance();

    Language language() const;
    QWidget *createSelector(QWidget *parent = nullptr);
    QString translate(const QString &source) const;
    QString sourceText(const QString &displayText) const;
    QString comboValue(const QComboBox *combo) const;
    void setComboValue(QComboBox *combo, const QString &sourceValue) const;
    void retranslateAll();

public slots:
    void setLanguage(Language language);

signals:
    void languageChanged(Language language);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit LanguageManager(QObject *parent = nullptr);
    void retranslateWidget(QWidget *widget);
    void retranslateCombo(QComboBox *combo);

    Language m_language = Language::Chinese;
};

#endif // LANGUAGEMANAGER_H
