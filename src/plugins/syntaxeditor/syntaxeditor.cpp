#include <QtGui>

#include "syntaxeditor.h"
#include "golanghighlighter.h"

SyntaxEditor::SyntaxEditor()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    isUntitled = true;
}

void SyntaxEditor::reload()
{
    QFile file(curFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("LiteIDE"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(curFile)
                             .arg(file.errorString()));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    setPlainText(codec->toUnicode(file.readAll()));
    QApplication::restoreOverrideCursor();
}

void SyntaxEditor::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("document%1.go").arg(sequenceNumber++);
    curText = curFile + "[*]";
    setWindowTitle(curText);
}

SyntaxEditor *SyntaxEditor::openFile(const QString &fileName)
{
    SyntaxEditor *editor = new SyntaxEditor;
    if (editor->loadFile(fileName)) {
        editor->setCurrentFile(fileName);
        return editor;
    } else {
        delete editor;
        return 0;
    }
}

bool SyntaxEditor::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("LiteIDE"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    setPlainText(codec->toUnicode(file.readAll()));
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);

    return true;
}

bool SyntaxEditor::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool SyntaxEditor::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool SyntaxEditor::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("LiteIDE"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    file.write(codec->fromUnicode(toPlainText()));
    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return true;
}

QString SyntaxEditor::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void SyntaxEditor::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool SyntaxEditor::maybeSave()
{
    if (document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("LiteIDE"),
                     tr("'%1' has been modified.\n"
                        "Do you want to save your changes?")
                     .arg(userFriendlyCurrentFile()),
                     QMessageBox::Save | QMessageBox::Discard
                     | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void SyntaxEditor::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    document()->setModified(false);
    setWindowModified(false);
    curText = userFriendlyCurrentFile();
}

QString SyntaxEditor::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}