#include "todostore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QtGlobal>

void TodoStore::load() {
  items_.clear();
  next_id_ = 1;
  QFile f(storePath());
  if (!f.open(QIODevice::ReadOnly)) return;
  const auto doc = QJsonDocument::fromJson(f.readAll());
  if (!doc.isArray()) return;
  for (const auto& v : doc.array()) {
    if (!v.isObject()) continue;
    const auto o = v.toObject();
    TodoItem it;
    it.id = static_cast<quint64>(o.value(QStringLiteral("id")).toDouble());
    it.text = o.value(QStringLiteral("text")).toString();
    it.done = o.value(QStringLiteral("done")).toBool();
    if (it.id == 0) it.id = next_id_++;
    if (it.id >= next_id_) next_id_ = it.id + 1;
    items_.push_back(it);
  }
}

void TodoStore::save() {
  QJsonArray arr;
  for (const auto& it : items_) {
    QJsonObject o;
    o.insert(QStringLiteral("id"), static_cast<double>(it.id));
    o.insert(QStringLiteral("text"), it.text);
    o.insert(QStringLiteral("done"), it.done);
    arr.append(o);
  }
  const QDir dir = QFileInfo(storePath()).absoluteDir();
  dir.mkpath(QStringLiteral("."));
  QFile f(storePath());
  if (!f.open(QIODevice::WriteOnly)) return;
  f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

QString TodoStore::storePath() {
  QString base = qEnvironmentVariable("APPDATA");
  if (base.isEmpty())
    base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  else
    base = base + QStringLiteral("/todo-qt");
  return base + QStringLiteral("/todos.json");
}

void TodoStore::add(const QString& text) {
  const auto t = text.trimmed();
  if (t.isEmpty()) return;
  items_.push_back({next_id_++, t, false});
  save();
}

void TodoStore::addDone(const QString& text) {
  const auto t = text.trimmed();
  if (t.isEmpty()) return;
  items_.push_back({next_id_++, t, true});
  save();
}

void TodoStore::setDone(int index, bool done) {
  if (index < 0 || index >= static_cast<int>(items_.size())) return;
  if (items_[index].done == done) return;
  items_[index].done = done;
  save();
}

void TodoStore::removeAt(int index) {
  if (index < 0 || index >= static_cast<int>(items_.size())) return;
  items_.erase(items_.begin() + index);
  save();
}

void TodoStore::moveItem(int from, int to) {
  if (from < 0 || from >= static_cast<int>(items_.size())) return;
  if (to < 0 || to >= static_cast<int>(items_.size())) return;
  if (from == to) return;
  const auto item = items_[from];
  items_.erase(items_.begin() + from);
  items_.insert(items_.begin() + to, item);
  save();
}

void TodoStore::updateText(int index, const QString& text) {
  const auto t = text.trimmed();
  if (t.isEmpty()) return;
  if (index < 0 || index >= static_cast<int>(items_.size())) return;
  items_[index].text = t;
  save();
}

void TodoStore::importMarkdown(const QString& content) {
  for (const auto& raw : content.split(QRegularExpression(QStringLiteral("[\\r\\n]+")))) {
    const auto line = raw.trimmed();
    if (line.isEmpty()) continue;
    if (line.startsWith(QStringLiteral("- [x]"), Qt::CaseInsensitive))
      addDone(line.mid(5).trimmed());
    else if (line.startsWith(QStringLiteral("- [ ]")))
      add(line.mid(5).trimmed());
    else
      add(line);
  }
}

QString TodoStore::exportMarkdown() const {
  QStringList lines;
  for (const auto& it : items_) {
    lines << (it.done ? QStringLiteral("- [x] %1").arg(it.text)
                      : QStringLiteral("- [ ] %1").arg(it.text));
  }
  return lines.join(QLatin1Char('\n'));
}

QString TodoStore::countsLabel() const {
  int left = 0, done = 0;
  for (const auto& it : items_) {
    if (it.done) ++done;
    else ++left;
  }
  return QStringLiteral("还剩 %1 · 已完成 %2").arg(left).arg(done);
}

std::vector<int> TodoStore::filteredIndices(FilterKind kind) const {
  std::vector<int> r;
  for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
    const auto& it = items_[i];
    if (kind == FilterKind::Active && it.done) continue;
    if (kind == FilterKind::Done && !it.done) continue;
    r.push_back(i);
  }
  return r;
}

void TodoStore::replaceFromJsonFile(const QString& path) {
  QFile in(path);
  if (!in.open(QIODevice::ReadOnly)) return;
  QFile out(storePath());
  QDir().mkpath(QFileInfo(storePath()).absolutePath());
  if (!out.open(QIODevice::WriteOnly)) return;
  out.write(in.readAll());
  load();
}
