#include "c_newseed_w.h"
#include "ui_c_newseed.h"
#include "../state/c_newseedshow.h"
#include "../util/widgetutils.h"
#include "../control/messagebox.h"

namespace wnd {


NewSeed::NewSeed( QWidget *parent, state::SubmitCaller * _state, state::StateMachine * stateMachine, const QVector<QString> & _seed, bool hideSubmitButton ) :
    core::NavWnd( parent, stateMachine, !hideSubmitButton ),
    ui(new Ui::NewSeed),
    state(_state),
    seed(_seed)
{
    ui->setupUi(this);

    if ( hideSubmitButton )
        ui->submitButton->hide();
    else
        utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );

    if (!seed.isEmpty()) {
        updateSeedData(seed);
    }

}

NewSeed::~NewSeed()
{
    delete ui;
}

// if seed empty or has size 1, it is error message
void NewSeed::showSeedData(const QVector<QString> & seed) {
    if (seed.size()<2) {
        control::MessageBox::message( this, "Getting Seed Failure", "Unable to retrieve a seed from mwc713." + (seed.size()>0 ? seed[0] : "") );
        return;
    }

    updateSeedData(seed);
}

void NewSeed::updateSeedData(const QVector<QString> & seed) {
    QString thePhrase = "";
    for (const auto & s : seed) {
        if (thePhrase.length()>0)
            thePhrase+=" ";
        thePhrase+=s;
    }
    ui->seedText->setPlainText( "Seed:\n" + thePhrase);
    ui->seedText->setFocus();
}

void NewSeed::on_submitButton_clicked()
{
    state->submit();
}

}
