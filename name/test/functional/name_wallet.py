#!/usr/bin/env python3
# Copyright (c) 2014-2017 Daniel Kraft
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# RPC tests for the handling of names in the wallet.

from test_framework.names import NameTestFramework
from test_framework.util import *

from decimal import Decimal

nameFee = Decimal ("0.01")
txFee = Decimal ("0.001")
initialBalance = Decimal ("1250")
zero = Decimal ("0")

class NameWalletTest (NameTestFramework):

  spentA = zero
  spentB = zero

  def set_test_params (self):
    # Set paytxfee to an explicitly known value.
    self.setup_name_test ([["-paytxfee=%s" % txFee]] * 4)

  def getFee (self, ind, txid, extra = zero):
    """
    Return and check the fee of a transaction.  There may be an additional
    fee for the locked coin, and the paytxfee times the tx size.
    The tx size is queried from the node with the given index by the txid.
    """

    info = self.nodes[ind].gettransaction (txid)
    totalFee = -info['fee']
    assert totalFee >= extra

    absFee = totalFee - extra
    size = Decimal (len (info['hex']) / 2)
    assert_fee_amount (absFee, size, txFee)

    return totalFee

  def checkBalance (self, ind, spent):
    """
    Check the balance of the node with index ind.  It should be
    the initial balance minus "spent".
    """

    bal = self.nodes[ind].getbalance ()
    assert_equal (bal, initialBalance - spent)

    assert_equal (self.nodes[ind].getbalance (), bal)
    assert_equal (self.nodes[ind].getbalance (""), bal)
    assert_equal (self.nodes[ind].getbalance ("*"), bal)
    assert_equal (self.nodes[ind].listaccounts (), {"":bal})

  def checkBalances (self, spentA = zero, spentB = zero):
    """
    Check balances of nodes 2 and 3.  The expected spent amounts
    for them are stored in self.spentA and self.spentB, and increased
    prior to the check by the arguments passed in.
    """

    self.spentA += spentA
    self.spentB += spentB

    self.checkBalance (2, self.spentA)
    self.checkBalance (3, self.spentB)

  def checkTx (self, ind, txid, amount, fee, details):
    """
    Call 'gettransaction' and compare the result to the
    expected data given in the arguments.  "details" is an array
    containing all the tx sent/received entries expected.
    Each array element is an array itself with the fields:

      [category, nameop, amount, fee]

    nameop can be None if no "name" key is expected.
    """

    data = self.nodes[ind].gettransaction (txid)
    assert_equal (data['amount'], amount)

    if fee is None:
      assert 'fee' not in data
    else:
      assert_equal (data['fee'], fee)

    # Bring the details returned in the same format as our expected
    # argument.  Furthermore, check that each entry has an address
    # set (but don't compare the address, since we don't know it).
    detailsGot = []
    for d in data['details']:
      assert 'address' in d
      if 'name' in d:
        nameOp = d['name']
        if nameOp[:3] == 'new':
          nameOp = 'new'
      else:
        # None is not sortable in Python3, so use "none" instead.
        nameOp = "none"
      if 'fee' in d:
        fee = d['fee']
      else:
        fee = None
      detailsGot.append ([d['category'], nameOp, d['amount'], fee])

    # Compare.  Sort to get rid of differences in the order.
    detailsGot.sort ()
    details.sort ()
    assert_equal (detailsGot, details)

  def run_test (self):
    # Note that the next 50 maturing blocks will be for nodes 0 and 1.
    # Thus we use 2 and 3 for the tests, because their balance
    # will stay constant over time except for our explicit transactions.

    self.checkBalances ()

    # Register and update a name.  Check changes to the balance.
    newA = self.nodes[2].name_new ("name-a")
    newFee = self.getFee (2, newA[0], nameFee)
    self.generate (0, 5)
    self.checkBalances (newFee)
    firstA = self.firstupdateName (2, "name-a", newA, "value")
    firstFee = self.getFee (2, firstA)
    self.generate (0, 10)
    self.checkBalances (firstFee)
    updA = self.nodes[2].name_update ("name-a", "new value")
    updFee = self.getFee (2, updA)
    self.generate (0, 1)
    self.checkBalances (updFee)

    # Check the transactions.
    self.checkTx (2, newA[0], zero, -newFee,
                  [['send', 'new', zero, -newFee]])
    self.checkTx (2, firstA, zero, -firstFee,
                  [['send', 'update: name-a', zero, -firstFee]])
    self.checkTx (2, updA, zero, -updFee,
                  [['send', 'update: name-a', zero, -updFee]])

    # Send a name from 1 to 2 by firstupdate and update.
    addrB = self.nodes[3].getnewaddress ()
    newB = self.nodes[2].name_new ("name-b")
    fee = self.getFee (2, newB[0], nameFee)
    newC = self.nodes[2].name_new ("name-c")
    fee += self.getFee (2, newC[0], nameFee)
    self.generate (0, 5)
    self.checkBalances (fee)
    firstB = self.firstupdateName (2, "name-b", newB, "value", addrB)
    fee = self.getFee (2, firstB)
    firstC = self.firstupdateName (2, "name-c", newC, "value")
    fee += self.getFee (2, firstC)
    self.generate (0, 10)
    self.checkBalances (fee)
    updC = self.nodes[2].name_update ("name-c", "new value", addrB)
    fee = self.getFee (2, updC)
    self.generate (0, 1)
    self.checkBalances (fee)

    # Check the receiving transactions on B.
    self.checkTx (3, firstB, zero, None,
                  [['receive', 'update: name-b', zero, None]])
    self.checkTx (3, updC, zero, None,
                  [['receive', 'update: name-c', zero, None]])

    # Use the rawtx API to build a simultaneous name update and currency send.
    # This is done as an atomic name trade.  Note, though, that the
    # logic is a bit confused by "coin join" transactions and thus
    # possibly not exactly what one would expect.

    price = Decimal ("1.0")
    fee = Decimal ("0.01")
    txid = self.atomicTrade ("name-a", "enjoy", price, fee, 2, 3)
    self.generate (0, 1)

    self.checkBalances (-price, price + fee)
    self.checkTx (2, txid, price, None,
                  [['receive', "none", price, None]])
    self.checkTx (3, txid, -price, -fee,
                  [['send', "none", -price, -fee],
                   ['send', 'update: name-a', zero, -fee]])

    # Test sendtoname RPC command.

    addrDest = self.nodes[2].getnewaddress ()
    newDest = self.nodes[0].name_new ("destination")
    self.generate (0, 5)
    self.firstupdateName (0, "destination", newDest, "value", addrDest)
    self.generate (0, 10)
    self.checkName (3, "destination", "value", None, False)

    assert_raises_rpc_error (-5, 'name not found',
                             self.nodes[3].sendtoname, "non-existant", 10)

    txid = self.nodes[3].sendtoname ("destination", 10)
    fee = self.getFee (3, txid)
    self.generate (0, 1)
    self.checkBalances (-10, 10 + fee)

    txid = self.nodes[3].sendtoname ("destination", 10, "foo", "bar", True)
    fee = self.getFee (3, txid)
    self.generate (0, 1)
    self.checkBalances (-10 + fee, 10)

    self.generate (0, 30)
    self.checkName (3, "destination", "value", None, True)
    assert_raises_rpc_error (-5, 'the name is expired',
                             self.nodes[3].sendtoname, "destination", 10)

if __name__ == '__main__':
  NameWalletTest ().main ()
