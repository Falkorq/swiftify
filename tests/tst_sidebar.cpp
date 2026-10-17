#include "ViewRig.h"
#include "app/Fonts.h"
#include <QQmlComponent>
#include <QtQml/qqmlextensionplugin.h>
Q_IMPORT_QML_PLUGIN(SwiftifyPlugin)
using namespace swiftify;
class SidebarTest : public QObject {
    Q_OBJECT
private slots:
    void toggleAndRestore() {
        fonts::loadIconFont();
        fonts::loadInterfaceFont();
        testing::ViewRig rig;
        rig.settings.setReduceMotion(true);
        rig.session.restoreSavedSessions();
        QTRY_VERIFY(!rig.session.isEvaluatingStoredLogin());
        QQmlComponent component(&rig.engine);
        component.loadFromModule("Swiftify", "Main");
        std::unique_ptr<QObject> main(component.create());
        QVERIFY2(main, qPrintable(component.errorString()));
        auto *window = qobject_cast<QQuickWindow *>(main.get());
        QVERIFY(window);
        auto *sidebar = main->findChild<QQuickItem *>("mainSidebar");
        auto *button = main->findChild<QQuickItem *>("sidebarCollapseButton");
        auto *sheet = main->findChild<QQuickItem *>("mainSheet");
        QVERIFY(sidebar && button && sheet);
        const qreal expanded = sidebar->width();
        const qreal sheetExpanded = sheet->width();
        QVERIFY(expanded >= 180);
        QVERIFY(QMetaObject::invokeMethod(button, "clicked"));
        QTRY_VERIFY(rig.settings.sidebarCollapsed());
        QTRY_COMPARE(sidebar->width(), 72.0);
        QVERIFY(sheet->width() > sheetExpanded);
        QVERIFY(button->property("tooltip").toString().startsWith("Expand sidebar"));
        QVERIFY(button->property("text").toString().isEmpty());
        {
            Settings restored(rig.directory.filePath("swiftify.conf"));
            QVERIFY(restored.sidebarCollapsed());
        }
        window->show();
        QTest::keyClick(window, Qt::Key_B, Qt::ControlModifier);
        QTRY_VERIFY(!rig.settings.sidebarCollapsed());
        QTRY_COMPARE(sidebar->width(), expanded);
        QVERIFY(QMetaObject::invokeMethod(button, "clicked"));
        QTRY_COMPARE(sidebar->width(), 72.0);
        QTest::qWait(200);
        const QImage frame = window->grabWindow();
        const QString previewPath = qEnvironmentVariable("SWIFTIFY_SIDEBAR_PREVIEW");
        if (!previewPath.isEmpty() && !frame.isNull()) QVERIFY(frame.save(previewPath));
    }
};
QTEST_MAIN(SidebarTest)
#include "tst_sidebar.moc"
