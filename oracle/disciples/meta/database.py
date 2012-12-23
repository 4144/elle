# -*- encoding: utf-8 -*-

import pymongo
try:
    from pymongo.objectid import ObjectId
except ImportError:
    from bson.objectid import ObjectId

from . import conf

"""
Define database collections and their constraints
"""

_connection = None
def connection(host="localhost", port=27017):
    global _connection
    if _connection is None:
        _connection = pymongo.Connection(host, port)
    return _connection

_database = None
def database(conn=None):
    global _database
    if conn is not None:
        return conn.meta
    if _database is None:
        _database = getattr(connection(), conf.COLLECTION_NAME)
    return _database

# collections
_users = None
def users(conn=None):
    global _users
    if _users is None:
        _users = database(conn)['users']
        # constraints
        _users.ensure_index(
            'email', pymongo.ASCENDING,
            kwags={'unique': True}
        )
    return _users

_devices = None
def devices(conn=None):
    global _devices
    if _devices is None:
        _devices = database(conn)['devices']
    return _devices

_sessions = None
def sessions(conn=None):
    global _sessions
    if _sessions is None:
        _sessions = database(conn)['sessions']
    return _sessions

_networks = None
def networks(conn=None):
    global _networks
    if _networks is None:
        _networks = database(conn)['networks']
    return _networks

_invitations = None
def invitations(conn=None):
    global _invitations
    if _invitations is None:
        _invitations = database(conn)['invitations']
    return _invitations

_transactions = None
def transactions(conn=None):
    global _transactions
    if _transactions is None:
        _transactions = database(conn)['transactions']
    return _transactions

# Collection that keep transaction history.
_finished_transactions = None
def finished_transactions(conn=None):
    global _finished_transactions
    if _finished_transactions is None:
        _finished_transactions = database(conn)['finished_transactions']
    return _finished_transactions

_notifications = None
def notifications(conn=None):
    global _notifications
    if _notifications is None:
        _notifications = database(conn)['notifications']
    return _notifications

# functions
def byId(collection, _id):
    """
    Get an object from collection `collection' with its id `_id'
    """
    return collection.find_one({'_id': ObjectId(_id)})
