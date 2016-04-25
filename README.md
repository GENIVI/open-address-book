# open-address-book
Open Address Book (OAB) -- sync external address books with your IVI system

Open Address Book is a flexible, modular component that services dedicated IVI use cases around the intelligent synchronization of external address books with a head-unit's contact management subsystem. Intelligence here revolves around particular needs in the area of flash wear out to large address book synchronization.

OpenAB is a set of helper libraries that can be used to implement any PIM functionalities.
It is a Plugin based library. New functionalities/customizations may be implemented using its plugin mechanis.

Currently supported:
* Contacts sources: PBAP, CardDAV, Evolution Data Server
* Calendar sources: CalDAV, Evolution Data Server
* Synchronization: One way (from server to local), Two way (from server to local and from local to server)

Executing Tests
* In order to execute the examples correctly, it is nessisary to setup the icloud/google account details.
** Please replace all reference to  test.emailaddress@gmail.com with valid google account
** Please replace all reference to  test.emailaddress_password with valid google account password

* For CalDAV please see :- https://developers.google.com/google-apps/calendar/caldav/v2/guide
* Please replace all reference to caldav_client_id.apps.googleusercontent.com with valid caldav client id
* Please replace all reference to caldav_client_secret with a valid caldav secret
* Please replace all reference to caldav_refreshtoken  with a valid caldav refresh token

