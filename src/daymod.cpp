#include "daymod.h"

DayMod::DayMod(QWidget* parent, Ui::MainWindow* mainUi) : QWidget(parent), ui(mainUi) 
{
    if(!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        QMessageBox::critical(this, "错误", "SQLite驱动未加载");
        return;
    }
    
    dayDb = QSqlDatabase::addDatabase("QSQLITE", "daymod_connection"); 
    // 自动搜索路径列表
    QStringList dbPaths = { 
        "day.db",  // 当前目录
        "../day.db",  // 上级目录
        "data/day.db"  // 下级data目录
    };
    
    bool dbOpened = false;
    for(const auto& path : dbPaths) {
        dayDb.setDatabaseName(path);
        if(dayDb.open()) {
            dbOpened = true;
            break;
        }
    }
    
    if(!dbOpened) {
        // 让用户选择数据库文件位置
        QString fileName = QFileDialog::getSaveFileName(this, 
            tr("选择或创建数据库文件"), 
            QDir::currentPath(), 
            tr("SQLite数据库文件 (*.db)"));
        
        if(fileName.isEmpty()) {
            QMessageBox::warning(this, "错误", "必须选择数据库文件位置");
            return;
        }
        
        dayDb.setDatabaseName(fileName);
        if(!dayDb.open()) {
            QMessageBox::warning(this, "数据库打开失败", dayDb.lastError().text());
            return;
        }
    }
    // 检查表是否存在
    if(!dayDb.tables().contains("important_days")) {
        QSqlQuery query(dayDb);
        
        // 创建主表
        QString createTable = "CREATE TABLE important_days ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "event_name TEXT NOT NULL, "
            "event_date TEXT NOT NULL, "
            "remarks TEXT DEFAULT '', "
            "created_at DATETIME DEFAULT (datetime('now','localtime')), " 
            "revision_at DATETIME DEFAULT (datetime('now','localtime')), "
            "UNIQUE(event_name, event_date) ON CONFLICT REPLACE)";
        if(!query.exec(createTable)) {
            QMessageBox::warning(this, "创建表失败", query.lastError().text());
            return;
        }

        // 创建触发器
        // 修改触发器使用本地时间
        // 修改触发器使用+8时区
        QStringList triggers = {
            "CREATE TRIGGER update_revision_time AFTER UPDATE ON important_days FOR EACH ROW "
            "BEGIN UPDATE important_days SET revision_at = datetime('now','localtime') WHERE id = OLD.id; END",
            
            "CREATE TRIGGER validate_date_format BEFORE INSERT ON important_days FOR EACH ROW "
            "BEGIN SELECT CASE WHEN date(NEW.event_date) IS NULL THEN RAISE(ABORT, 'Invalid date value') END; END",
            
            "CREATE TABLE operation_logs ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "event_id INTEGER NOT NULL, "
            "change_type TEXT NOT NULL CHECK(change_type IN ('INSERT', 'DELETE', 'UPDATE')), "
            "change_time TEXT DEFAULT (datetime('now','localtime')), "
            "change_details TEXT, "
            "FOREIGN KEY(event_id) REFERENCES important_days(id))",
            
            "CREATE TRIGGER log_insert AFTER INSERT ON important_days "
            "BEGIN "
            "INSERT INTO operation_logs (event_id, change_type, change_details) "
            "VALUES (NEW.id, 'INSERT', '新增事件: ' || NEW.event_name || ' 日期: ' || NEW.event_date); "
            "END",
            
            "CREATE TRIGGER log_update AFTER UPDATE ON important_days "
            "BEGIN "
            "INSERT INTO operation_logs (event_id, change_type, change_details) "
            "VALUES (NEW.id, 'UPDATE', "
            "TRIM('事件修改: ' || "
            "CASE WHEN OLD.event_name IS NOT NEW.event_name THEN '名称: ' || OLD.event_name || '→' || NEW.event_name || '; ' ELSE '' END || "
            "CASE WHEN OLD.event_date IS NOT NEW.event_date THEN '日期: ' || OLD.event_date || '→' || NEW.event_date || '; ' ELSE '' END || "
            "CASE WHEN OLD.remarks IS NOT NEW.remarks THEN '备注: ' || IFNULL(OLD.remarks,'NULL') || '→' || IFNULL(NEW.remarks,'NULL') ELSE '' END, "
            "' ;')); "
            "END",
            
            "CREATE TRIGGER log_delete AFTER DELETE ON important_days "
            "BEGIN "
            "INSERT INTO operation_logs (event_id, change_type, change_details) "
            "VALUES (OLD.id, 'DELETE', '删除事件: ' || OLD.event_name || ' 日期: ' || OLD.event_date); "
            "END",
            
            "CREATE TRIGGER auto_clean_logs AFTER INSERT ON important_days WHEN strftime('%d', 'now') = '01' "
            "BEGIN DELETE FROM operation_logs WHERE julianday('now') - julianday(change_time) > 30; END"
        };

        for(const auto& trigger : triggers) {
            if(!query.exec(trigger)) {
                QMessageBox::warning(this, "创建触发器失败", query.lastError().text());
                return;
            }
        }
        QMessageBox::information(this, "初始化成功", "数据库初始化成功"); 
    }   

    // 初始化模型
    model = new QSqlTableModel(this, dayDb);
    model->setTable("important_days");  // 设置表名
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);  // 手动提交修改
    
    // 设置按事件日期降序排序
    model->setSort(2, Qt::AscendingOrder);  // 第2列是event_date
    model->select();  // 加载数据

    // 设置表头
    model->setHeaderData(1, Qt::Horizontal, tr("事件名称"));
    model->setHeaderData(2, Qt::Horizontal, tr("事件日期")); 
    model->setHeaderData(3, Qt::Horizontal, tr("备注"));
    model->setHeaderData(4, Qt::Horizontal, tr("创建时间"));
    model->setHeaderData(5, Qt::Horizontal, tr("更新时间"));
    ui->dayTableView->setModel(model);
    ui->dayTableView->hideColumn(0); 
    
    // 设置自定义委托
    ui->dayTableView->setItemDelegate(new BoolDelegate(this, true)); // 为整个表格设置委托

    // 获取水平表头并设置列宽
    QHeaderView* header = ui->dayTableView->horizontalHeader();
    // 设置列宽调整模式（可选）
    header->setSectionResizeMode(QHeaderView::Interactive); // 允许手动调整

    // 设置各列初始宽度（单位：像素）
    header->resizeSection(1, 150);  // 事件名称 
    header->resizeSection(2, 150);  // 事件日期
    header->resizeSection(3, 200);  // 备注
    header->resizeSection(4, 150);  // 创建时间
    header->resizeSection(5, 150);  // 更新时间
    updateFirstEventMessage();  // 刷新后更新显示
}

void DayMod::refreshDayTable()
{
    model->select();
    updateFirstEventMessage();  // 刷新后更新显示
}

void DayMod::commitDayTable()
{
    if(!model) {
        QMessageBox::warning(this, "错误", "数据模型未初始化");
        return;
    }

    // 开始事务
    dayDb.transaction();
    
    // 提交所有修改
    if(!model->submitAll()) {
        dayDb.rollback();
        QMessageBox::warning(this, "提交失败", model->lastError().text());
        return;
    }
    
    // 提交事务
    if(!dayDb.commit()) {
        QMessageBox::warning(this, "提交失败", dayDb.lastError().text());
        return;
    }
    
    QMessageBox::information(this, "成功", "数据已提交");
    refreshDayTable();  // 刷新显示
}

void DayMod::deleteDayTable()
{
    if(!model) {
        QMessageBox::warning(this, "错误", "数据模型未初始化");
        return;
    }

    // 获取选中的行（包括单元格选择的情况）
    QModelIndexList selectedIndexes = ui->dayTableView->selectionModel()->selectedIndexes();
    if(selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的行");
        return;
    }

    // 提取所有选中的行号（去重）
    QSet<int> rowsToDelete;
    for(const auto& index : selectedIndexes) {
        rowsToDelete.insert(index.row());
    }

    // 确认删除
    if(QMessageBox::question(this, "确认删除", "确定删除选中的行吗？") != QMessageBox::Yes) {
        return;
    }

    // 开始事务
    dayDb.transaction();
    
    // 按行号从大到小删除（避免行号变化导致的问题）
    QList<int> sortedRows = rowsToDelete.values();
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
    
    for(int row : sortedRows) {
        model->removeRow(row);
    }
    
    if(!model->submitAll()) {
        dayDb.rollback();
        QMessageBox::warning(this, "删除失败", model->lastError().text());
        return;
    }
    
    if(!dayDb.commit()) {
        QMessageBox::warning(this, "删除失败", dayDb.lastError().text());
        return;
    }
}

void DayMod::insertDayTable()
{
    if(!model) {
        QMessageBox::warning(this, "错误", "数据模型未初始化");
        return;
    }

    // 新增一行并设置默认值
    int row = model->rowCount();
    if(!model->insertRow(row)) {
        QMessageBox::warning(this, "错误", "无法插入新行");
        return;
    }
    
    // 设置默认值
    QModelIndex nameIndex = model->index(row, 1); // 事件名称列
    QModelIndex dateIndex = model->index(row, 2); // 事件日期列
    QModelIndex remarksIndex = model->index(row, 3); // 备注列
    
    model->setData(nameIndex, tr("新事件"));
    model->setData(dateIndex, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")); // 直接使用当前时间
    model->setData(remarksIndex, tr(""));
    
    // 滚动到新行并选中
    ui->dayTableView->scrollToBottom();
    ui->dayTableView->setCurrentIndex(nameIndex);
    
    ui->dayTableView->viewport()->update();
}

void DayMod::cancelDayTable()
{
    if(!model) {
        QMessageBox::warning(this, "错误", "数据模型未初始化");
        return;
    }
    model->revertAll();
    QMessageBox::information(this,"取消","已取消更改");
}


DayMod::~DayMod()
{
}

BoolDelegate::BoolDelegate(QObject* parent, bool enableGrayEffect)
    : QStyledItemDelegate(parent), m_enableGrayEffect(enableGrayEffect)
{
}

void BoolDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // 获取事件日期
    QModelIndex dateIndex = index.sibling(index.row(), 2);
    QDateTime eventDate = QDateTime::fromString(dateIndex.data().toString(), "yyyy-MM-dd HH:mm:ss");
    int daysToEvent = QDateTime::currentDateTime().daysTo(eventDate);
    
    // 设置样式选项
    QStyleOptionViewItem opt = option;
    opt.displayAlignment = Qt::AlignCenter;
    initStyleOption(&opt, index);
    
    // 根据距离事件的天数设置不同颜色
    if (daysToEvent < 0) {
        opt.palette.setColor(QPalette::Text, Qt::gray);
    }
    else if (daysToEvent <= 1)
    {
        opt.palette.setColor(QPalette::Text, Qt::red);
    }
    else if (daysToEvent <= 3) {
        opt.palette.setColor(QPalette::Text, Qt::yellow);
    }
    else if (daysToEvent <= 7) {
        opt.palette.setColor(QPalette::Text, Qt::green);
    }
    else {
        opt.palette.setColor(QPalette::Text, Qt::white);
    }
    
    // 调用基类绘制
    QStyledItemDelegate::paint(painter, opt, index);
}


void DayMod::updateFirstEventMessage()
{
    if(!model || model->rowCount() == 0) {
        ui->dayMessageLabel->setText("暂无待办事项");
        ui->dayHomeLabel->setText("暂无待办事项");
        return;
    }

    int daysLeft = 0;
    QString eventName;
    QDateTime eventDate;

    for(int i = 0; i < model->rowCount(); i++)
    {
        QSqlRecord record = model->record(i);
        eventName = record.value("event_name").toString();
        eventDate = QDateTime::fromString(record.value("event_date").toString(), "yyyy-MM-dd HH:mm:ss");
    
        // 计算剩余天数
        daysLeft = QDateTime::currentDateTime().daysTo(eventDate);
        if(daysLeft >= 0)
            break;
    }


    
    // 设置显示文本
    QString message;
    if(daysLeft == 0) {
        message = QString("今天有活动: %1").arg(eventName);
    } else {
        message = QString("距离 %1 还有 %2 天 (%3)")
                     .arg(eventName)
                     .arg(daysLeft)
                     .arg(eventDate.toString("yyyy-MM-dd"));
    }
    
    ui->dayMessageLabel->setText(message);
    ui->dayHomeLabel->setText(message);
}