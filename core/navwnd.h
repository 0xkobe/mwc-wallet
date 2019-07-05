#ifndef NAVWND_H
#define NAVWND_H

#include <QWidget>

namespace state {
class StateMachine;
}

namespace core {

class NavBar;
class NavMenuAccount;
class AppContext;

// Window with navigation bar
class NavWnd : public QWidget {
Q_OBJECT
public:
    explicit NavWnd(QWidget *parent, state::StateMachine * stateMachine,
            AppContext * appContext, bool createNavigationButtons=true);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    NavBar * topRightButtonWnd = nullptr;
};

}

#endif // NAVWND_H
