#include "transaction.h"

trans_input::trans_input()
{
    clear();
}

bool trans_input::empty()
{
    return n == 0;
}

void trans_input::clear()
{
    pre_trans.clear();
    n = 0;
    pubkey.clear();
    sig.clear();
}

//////////////////////////////////////////////////////////////////////////

trans_output::trans_output()
{
    clear();
}

bool trans_output::empty()
{
    return n == 0;
}

void trans_output::clear()
{
    n = 0;
    pubkey.clear();
}

//////////////////////////////////////////////////////////////////////////

transaction::transaction()
{
    clear();
}

bool transaction::empty()
{
    return output.empty();
}

void transaction::clear()
{
    version = 0;
    input.clear();
    output.clear();
}