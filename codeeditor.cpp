#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QFontMetrics>
#include <QColor>
#include <QTextCursor>

// === LineNumberArea 类的实现 ===

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor), codeEditor(editor)
{
    // 构造函数将父部件设置为 editor，并保存 editor 指针
}

QSize LineNumberArea::sizeHint() const
{
    // 告诉布局管理器，行号区域的宽度由编辑器计算，高度自适应
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    // 这个函数会被 Qt 调用，用于绘制行号区域
    // 我们直接将绘制工作委托给 CodeEditor 的私有方法
    codeEditor->lineNumberAreaPaintEvent(event);
}

// === CodeEditor 类的实现 ===

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    // 1. 创建行号区域部件
    lineNumberArea = new LineNumberArea(this);

    // 2. 连接信号与槽，以实现动态更新
    // 当文档的块（行）数量变化时，更新行号区域的宽度
    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);

    // 当视图需要更新时（例如滚动），更新行号区域的显示
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);

    // 当光标位置改变时，高亮当前行
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    // 3. 初始化
    updateLineNumberAreaWidth(0); // 初始计算行号区域宽度
    highlightCurrentLine();       // 初始高亮当前行
}

int CodeEditor::lineNumberAreaWidth()
{
    // 计算显示所有行号所需的宽度
    int digits = 1;
    int max_line = qMax(1, blockCount()); // 获取最大行号

    // 计算最大行号的位数
    while (max_line >= 10) {
        max_line /= 10;
        digits++;
    }

    // 计算宽度：数字宽度 + 左右边距
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    // 设置视图的左边距，为行号区域留出空间
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy != 0) {
        // 如果是垂直滚动，直接滚动行号区域
        lineNumberArea->scroll(0, dy);
    } else {
        // 否则，更新指定区域
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    // 如果视图矩形包含了内容矩形，说明可能需要调整行号宽度
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    // 重写 resizeEvent 以响应窗口大小变化
    QPlainTextEdit::resizeEvent(e);

    // 获取内容区域的矩形
    QRect cr = contentsRect();
    // 设置行号区域的几何位置和大小
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    // 如果编辑器不是只读的
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        // 设置当前行高亮的背景色
        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true); // 整行选中

        QTextCursor cursor = textCursor();
        cursor.clearSelection(); // 清除可能存在的文本选择
        selection.cursor = cursor;

        extraSelections.append(selection);
    }

    // 应用额外的选择（即高亮当前行）
    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    // 这是实际绘制行号的函数

    QPainter painter(lineNumberArea);
    // 填充背景色
    painter.fillRect(event->rect(), Qt::lightGray);

    // 获取文档中第一个可见的块（行）
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber(); // 块号（从0开始）

    // 计算这个块在视图中的顶部和底部位置
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    // 遍历所有可见的块
    while (block.isValid() && top <= event->rect().bottom()) {
        // 如果块是可见的，并且其底部在绘制区域内
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1); // 转换为行号（从1开始）

            // 设置画笔颜色
            painter.setPen(Qt::black);

            // 绘制行号文本
            painter.drawText(0, top, lineNumberArea->width() - 3, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        // 移动到下一个块
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
