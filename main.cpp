#include "core/mainwindow.h"
#include <QApplication>
#include "core/windowmanager.h"
#include "wallet/mockwallet.h"
#include "state/state.h"
#include "state/statemachine.h"
#include "core/appcontext.h"
#include "util/ioutils.h"

int main(int argc, char *argv[])
{
    //Q_INIT_RESOURCE(completer);

    QApplication app(argc, argv);

    core::AppContext appContext;

    //main window has delete on close flag. That is why need to
    // create dynamically. Window will be deleted on close
    core::MainWindow * mainWnd = new core::MainWindow(nullptr);

    wallet::MockWallet wallet;

    core::WindowManager wndManager( mainWnd->getMainWindow() );

    mainWnd->show();

    state::StateContext context( &appContext, &wallet, &wndManager, mainWnd );

    state::StateMachine machine(context);
    mainWnd->setStateMachine(&machine);
    machine.start();

    return app.exec();
}