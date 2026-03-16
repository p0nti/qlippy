#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QTimer>
#include <functional>

#include "storage/Item.h"

class HistoryRepository;

class ClipboardModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(int resultLimit READ resultLimit WRITE setResultLimit NOTIFY resultLimitChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        PreviewRole,
        TimestampRole,
        PinnedRole,
        TypeRole,
    };
    Q_ENUM(Roles)

    explicit ClipboardModel(HistoryRepository* repo, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString searchText() const { return m_searchText; }
    void setSearchText(const QString& text);
    int resultLimit() const { return m_resultLimit; }
    void setResultLimit(int limit);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE qint64 idAt(int row) const;
    Q_INVOKABLE QString typeAt(int row) const;
    Q_INVOKABLE QString fullTextAt(int row) const;
    Q_INVOKABLE QString imageDataUrlAt(int row) const;
    Q_INVOKABLE void activate(int row);
    Q_INVOKABLE void deleteAt(int row);
    Q_INVOKABLE void togglePinAt(int row);
    Q_INVOKABLE bool isPinnedAt(int row) const;

    void setCopyHandler(std::function<void(qint64)> handler) { m_copyHandler = std::move(handler); }

signals:
    void searchTextChanged();
    void resultLimitChanged();
    void countChanged();

private slots:
    void applySearch();

private:
    void rebuildFilter();

    HistoryRepository* m_repo;
    QList<Item> m_all;
    QList<int> m_visible;
    QString m_searchText;
    int m_resultLimit = 500;
    QTimer m_debounce;
    std::function<void(qint64)> m_copyHandler;
};
