#!/usr/bin/python

import os
import dbus
import dbus.service
import time
from dbus.mainloop.glib import DBusGMainLoop
import gobject
import threading
import StringIO

dbus_bus_name = "org.bluez.obex"
obex_client1_interface = "org.bluez.obex.Client1"
obex_transfer1_interface = "org.bluez.obex.Transfer1"
obex_phonebook_access1_interface = "org.bluez.obex.PhonebookAccess1"
properties_interface = "org.freedesktop.DBus.Properties"

ongoingTransfers = []

vCardsNormal = """BEGIN:VCARD\n\r
VERSION:3.0\n\r
N:Surname;Name;Middle;Perfix;Suffix\n\r
FN:Prefix Name Middle Surname Suffix\n\r
ORG:Bubba Shrimp Co.\n\r
TITLE:Shrimp Man\n\r
PHOTO;VALUE=URI;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif\n\r
TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r
ADR;TYPE=WORK;TYPE=HOME,OFFICE:;;100 Waters Edge;Baytown;LA;0;United States of America\n\r
EMAIL;TYPE=PREF;TYPE=INTERNET:name.surname@example.com\n\r
GEO:39.95;-75.1667\n\r
BDAY:19700310\n\r
REV:123\n\r
UID:id1234\n\r
PRODID:OPENAB\n\r
X-EVOLUTION-LABEL:label\n\r
END:VCARD\n\r"""

vCardsMisformatted = """BEGIN:VCARD\n\r
VERSION:3.0\n\r
N:Surname;Name;Middle;Perfix;Suffix\n\r
FN:Prefix Name Middle Surname Suffix\n\r
TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r
END:VCARD\n\r
BEGIN:VCARD\n\r
VERSION:3.0\n\r
N:Surname2;Name2;Middle2;Perfix2;Suffix2\n\r
FN:Prefix2 Name2 Middle2 Surname2 Suffix2\n\r
PHOTO:http://www.example.com/dir_photos/my_photo.gif\n\r
END:VCARD\n\r
BEGIN:VCARD\n\r
VERSION:3.0\n\r
N:Surname3;Name3;Middle3;Perfix3;Suffix3\n\r
FN:Prefix3 Name3 Middle3 Surname3 Suffix3\n\r
TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r
END:VCARD\n\r"""


currentMAC = ""


class MockOBEXClient1(dbus.service.Object):
  def __init__(self):
    bus_name = dbus.service.BusName(dbus_bus_name, bus=dbus.SessionBus())
    dbus.service.Object.__init__(self, bus_name, "/org/bluez/obex")

  #Special use case - MAC address "DE:AD:DE:AD:DE:AD" will cause exception
  @dbus.service.method(dbus_interface=obex_client1_interface, in_signature="sa{sv}", out_signature="o")
  def CreateSession(self, dest, args):
    session_name = "/org/bluez/obex/session1"
    if(dest == "DE:AD:DE:AD:DE:AD"):
      raise dbus.exception.DBusException('org.bluez.obex.Error.Failed', 'Create Session with DE:AD:DE:AD:DE:AD test exception')

    global currentMAC
    currentMAC = dest
    return dbus.ObjectPath(session_name)

  @dbus.service.method(dbus_interface=obex_client1_interface, in_signature="s", out_signature="")
  def RemoveSession(self, dest):
    pass

class MockOBEXTransfer1(dbus.service.Object, threading.Thread):
  def __init__(self, path, targetFile, filters):
    bus_name = dbus.service.BusName(dbus_bus_name, bus=dbus.SessionBus())
    dbus.service.Object.__init__(self, bus_name, path)
    threading.Thread.__init__(self)
    self.targetFile = targetFile
    self.filters = filters
    self.paused = False
    self.cancelled = False
    self.start()

  #Special use case - for MAC 55:55:55:55:55:55 - this will fail
  @dbus.service.method(dbus_interface=obex_transfer1_interface, in_signature="", out_signature="")
  def Cancel(self):
    global currentMAC
    print("Cancelling " + currentMAC)
    if(currentMAC == "55:55:55:55:55:55"):
      raise dbus.exception.DBusException('org.bluez.obex.Error.Failed', 'Cancel Transfer with 55:55:55:55:55:55 test exception')

    if not self.cancelled:
      self.cancelled = True
    pass

  #Special use case - for MAC 55:55:55:55:55:55 - this will fail
  @dbus.service.method(dbus_interface=obex_transfer1_interface, in_signature="", out_signature="")
  def Suspend(self): 
    global currentMAC
    if(currentMAC == "55:55:55:55:55:55"):
      raise dbus.exception.DBusException('org.bluez.obex.Error.Failed', 'Suspend Transfer with 55:55:55:55:55:55 test exception')

    if not self.paused:
      self.paused = True
      self.PropertiesChanged(obex_transfer1_interface, dbus.Dictionary({"Status" : "suspended"}, signature="sv"), dbus.Array([], signature="s"))
    pass
 
  #Special use case - for MAC 55:55:55:55:55:55 - this will fail
  @dbus.service.method(dbus_interface=obex_transfer1_interface, in_signature="", out_signature="")
  def Resume(self):
    global currentMAC
    if(currentMAC == "55:55:55:55:55:55"):
      raise dbus.exception.DBusException('org.bluez.obex.Error.Failed', 'Resume Transfer with 55:55:55:55:55:55 test exception')

    if self.paused:
      self.paused = False
      self.PropertiesChanged(obex_transfer1_interface, dbus.Dictionary({"Status" : "active"}, signature="sv"), dbus.Array([], signature="s"))
    pass

  @dbus.service.signal(dbus_interface=properties_interface, signature="sa{sv}as")
  def PropertiesChanged(self, interface, changed_properties, invalidated_properties):
    return None

  #Special use case - for MAC 44:44:44:44:44:44 - this will send misformatted vcard
  def run(self):
    global currentMAC
    f = open(self.targetFile, 'w')
    buf = ""
    if (currentMAC == "44:44:44:44:44:44"):
      buf = StringIO.StringIO(vCardsMisformatted)
    else:
      buf = StringIO.StringIO(vCardsNormal)

    self.PropertiesChanged(obex_transfer1_interface, dbus.Dictionary({"Status" : "queued"}, signature="sv"), [])
    self.PropertiesChanged(obex_transfer1_interface, dbus.Dictionary({"Status" : "active"}, signature="sv"), [])
    while 1:
      if self.cancelled:
        self.PropertiesChanged(obex_transfer1_interface, dbus.Dictionary({"Status" : "error"}, signature="sv"), dbus.Array([], signature="s"))
        f.close()
        break

      if self.paused:
        continue

      data = buf.readline()
      if data=='':
        break

      if "Fields" in self.filters:
	if ((not ("PHOTO" in self.filters["Fields"]) and "PHOTO" in data) or
           (not ("ADR" in self.filters["Fields"]) and "ADR" in data) or
           (not ("TEL" in self.filters["Fields"]) and "TEL" in data)):
	  print("field ignored")
          continue

      f.write(data)
      time.sleep(0.01)
    f.close()
    self.PropertiesChanged(obex_transfer1_interface, dbus.Dictionary({"Status" : "complete"}, signature="sv"), dbus.Array([], signature="s"))

class MockOBEXPhonebookAccess1(dbus.service.Object):
  def __init__(self):
    bus_name = dbus.service.BusName(dbus_bus_name, bus=dbus.SessionBus())
    dbus.service.Object.__init__(self, bus_name, "/org/bluez/obex/session1")
    self.transferNum = 0

  #Special use case - sim3 location will cause exception
  @dbus.service.method(dbus_interface=obex_phonebook_access1_interface, in_signature="ss", out_signature="")
  def Select(self, location, phonebook):
    if(location == "sim3"):
      raise dbus.exception.DBusException('org.bluez.obex.Error.Failed', 'Select sim3 test exception')

  #Special use case - for MAC 11:11:11:11:11:11 - this will fail
  @dbus.service.method(dbus_interface=obex_phonebook_access1_interface, in_signature="", out_signature="as")
  def ListFilterFields(self):
    global currentMAC
    if(currentMAC == "11:11:11:11:11:11"):
      raise dbus.exception.DBusException('org.bluez.obex.Error.Failed', 'ListFilterFields test exception')
    return ['VERSION', 'FN', 'N', 'PHOTO', 'BDAY', 'ADR', 'LABEL', 'TEL', 'EMAIL', 'MAILER', 'TZ', 'GEO', 'TITLE', 'ROLE', 'LOGO', 'AGENT', 'ORG', 'NOTE', 'REV', 'SOUND', 'URL', 'UID', 'KEY', 'NICKNAME', 'CATEGORIES', 'PROID', 'CLASS']

  #Special use case - for MAC 22:22:22:22:22:22 - this will fail
  @dbus.service.method(dbus_interface=obex_phonebook_access1_interface, in_signature="", out_signature="q")
  def GetSize(self): 
    global currentMac
    if(currentMAC == "22:22:22:22:22:22"):
      raise dbus.exception.DBusException('org.bluez.obex.Error.Failed', 'GetSize test exception')
    return 1

  #Special use case - for MAC 33:33:33:33:33:33 - this will fail 
  @dbus.service.method(dbus_interface=obex_phonebook_access1_interface, in_signature="sa{sv}", out_signature="oa{sv}")
  def PullAll(self, targetFile, filters):
    global currentMac
    if(currentMAC == "33:33:33:33:33:33"):
      raise dbus.exception.DBusException('org.bluez.obex.Error.Failed', 'PullAll ADR test exception')

    transfer = "/org/bluez/obex/session1/transfer" + str(self.transferNum);
    self.transferNum = self.transferNum + 1
    print("Filters: ")
    print(filters)
    newTransfer = MockOBEXTransfer1(transfer, targetFile, filters)
    ongoingTransfers.append(transfer)
    return (dbus.ObjectPath(transfer), dbus.Dictionary({"Filename": targetFile}, signature='sv'))

gobject.threads_init()
DBusGMainLoop(set_as_default=True)
client = MockOBEXClient1()
phonebook = MockOBEXPhonebookAccess1()


loop = gobject.MainLoop()
loop.run()
