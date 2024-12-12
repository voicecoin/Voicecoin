#!/usr/bin/env python3
# Copyright (c) 2014-2017 Daniel Kraft
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# RPC test for multisig handling with names.

from test_framework.names import NameTestFramework
from test_framework.util import *

from decimal import Decimal
import binascii

class NameMultisigTest (NameTestFramework):

  def set_test_params (self):
    self.setup_name_test ()

  def run_test (self):
    # Construct a 2-of-2 multisig address shared between two nodes.
    pubkeyA = self.getNewPubkey (0)
    pubkeyB = self.getNewPubkey (1)
    p2sh = self.nodes[0].addmultisigaddress (2, [pubkeyA, pubkeyB])
    p2sh_ = self.nodes[1].addmultisigaddress (2, [pubkeyA, pubkeyB])
    assert_equal (p2sh, p2sh_)

    # Register a new name to that address.
    new = self.nodes[0].name_new ("name")
    self.generate (0, 10)
    self.firstupdateName (0, "name", new, "value", p2sh)
    self.generate (1, 5)
    data = self.checkName (2, "name", "value", None, False)
    assert_equal (data['address'], p2sh)

    # Straight-forward name updating should fail (for both nodes).
    assert_raises_rpc_error (-4, None,
                             self.nodes[0].name_update, "name", "new value")
    assert_raises_rpc_error (-4, None,
                             self.nodes[1].name_update, "name", "new value")

    # Find some other input to add as fee.
    unspents = self.nodes[0].listunspent ()
    assert len (unspents) > 0
    feeInput = unspents[0]
    changeAddr = self.nodes[0].getnewaddress ()
    nameAmount = Decimal ("0.01")
    changeAmount = feeInput['amount'] - nameAmount

    # Construct the name update as raw transaction.
    addr = self.nodes[2].getnewaddress ()
    inputs = [{"txid": data['txid'], "vout": data['vout']}, feeInput]
    outputs = {changeAddr: changeAmount, addr: nameAmount}
    txRaw = self.nodes[3].createrawtransaction (inputs, outputs)
    op = {"op": "name_update", "name": "name", "value": "it worked"}
    nameInd = self.rawtxOutputIndex (3, txRaw, addr)
    txRaw = self.nodes[3].namerawtransaction (txRaw, nameInd, op)

    # Sign it partially.
    partial = self.nodes[0].signrawtransaction (txRaw['hex'])
    assert not partial['complete']
    assert_raises_rpc_error (-26, None,
                             self.nodes[2].sendrawtransaction, partial['hex'])

    # Sign it fully and transmit it.
    signed = self.nodes[1].signrawtransaction (partial['hex'])
    assert signed['complete']
    tx = signed['hex']

    # Manipulate the signature to invalidate it.  This checks whether or
    # not the OP_MULTISIG is actually verified (vs just the script hash
    # compared to the redeem script).
    txData = bytearray (binascii.unhexlify (tx))
    txData[44] = (txData[44] + 10) % 256
    txManipulated = binascii.hexlify (txData).decode ("ascii")

    # Send the tx.  The manipulation should be caught (independently of
    # when strict P2SH checks are enabled, since they are enforced
    # mandatorily in the mempool).
    assert_raises_rpc_error (-26, None,
                             self.nodes[2].sendrawtransaction, txManipulated)
    self.nodes[2].sendrawtransaction (tx)
    self.generate (3, 1)

    # Check that it was transferred correctly.
    self.checkName (3, "name", "it worked", None, False)
    self.nodes[2].name_update ("name", "changed")
    self.generate (3, 1)
    self.checkName (3, "name", "changed", None, False)

  def getNewPubkey (self, ind):
    """
    Get a new address of one of the nodes and return directly
    the full pubkey.
    """

    addr = self.nodes[ind].getnewaddress ()
    data = self.nodes[ind].validateaddress (addr)

    return data['pubkey']

if __name__ == '__main__':
  NameMultisigTest ().main ()
