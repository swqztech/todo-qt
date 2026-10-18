#pragma once

#include <QJsonArray>
#include <QString>
#include <vector>

enum class FilterKind { All, Active, Done };

struct TodoItem {
  quint64 id = 0;
  QString text;
  bool done = false;
};

class TodoStore {
 public:
  void load();
  void save();

  void add(const QString& text);
  void addDone(const QString& text);
  void setDone(int index, bool done);
  void removeAt(int index);
  void moveItem(int from, int to);
  void updateText(int index, const QString& text);
  void importMarkdown(const QString& content);
  QString exportMarkdown() const;
  QString countsLabel() const;
  std::vector<int> filteredIndices(FilterKind kind) const;
  void replaceFromJsonFile(const QString& path);

  const std::vector<TodoItem>& items() const { return items_; }

 private:
  static QString storePath();

  std::vector<TodoItem> items_;
  quint64 next_id_ = 1;
};
