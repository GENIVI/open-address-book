/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_OpenAB__VCardID.cpp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <plugin/addressbook/Addressbook.hpp>
#include <plugin/input/Input.hpp>

#include <OpenAB.hpp>
#include <vcard/VCard.hpp>

typedef std::string       str;
typedef std::vector<str> vstr;

#define PV(_N,_A) \
  const char* _N ## _array = _A; \
  vstr _N(_N ## _array,sizeof(_N ## _array)/sizeof(_N ## _array[0]));

void testVC(
    str n1,
    str n2,
    const char** e1,
    const char** e2,
    const char** p1,
    const char** p2
    )
{
  printf("Name1:  %s\n",n1.c_str());
  printf("Name2:  %s\n",n2.c_str());

  printf("Email1:\n");
  vstr ve1;
  while (*e1){
    ve1.push_back(*e1);
    printf("        %s\n",*e1);
    ++e1;
  }
  printf("Email2:\n");
  vstr ve2;
  while (*e2){
    ve2.push_back(*e2);
    printf("        %s\n",*e2);
    ++e2;
  }

  printf("Phone1:\n");
  vstr vp1;
  while (*p1){
    vp1.push_back(*p1);
    printf("        %s\n",*p1);
    ++p1;
  }
  printf("Phone2:\n");
  vstr vp2;
  while (*p2){
    vp2.push_back(*p2);
    printf("        %s\n",*p2);
    ++p2;
  }


  OpenAB::VCardID vc1(n1,ve1,vp1);
  OpenAB::VCardID vc2(n2,ve2,vp2);

  if (vc1 == vc2){
    printf("vc1 == vc2 - EQUAL\n");
  }else{
    printf("vc1 != vc2 - NOT EQUAL\n");
  }
}


int main(int argc, char* argv[])
{

  OpenAB::OpenAB_init();

  int test = 0;

  {
    printf("\nTest %d\n",++test);
    str n1 = "Eugenio";
    str n2 = "Eugenio";
    const char* e1[] = {"e@e.it","b@b.it",NULL};
    const char* e2[] = {"e@e.it","b@b.it",NULL};
    const char* p1[] = {"e@e.it","b@b.it",NULL};
    const char* p2[] = {"e@e.it","b@b.it",NULL};
    testVC(n1,n2,e1,e2,p1,p2);
  }

  {
    printf("\nTest %d\n",++test);
    str n1 = "Eugenio";
    str n2 = "Eugenio";
    const char* e1[] = {"e@e.it","b@b.it",NULL};
    const char* e2[] = {"e@e.it","b@b.it",NULL};
    const char* p1[] = {"xxe@e.it","xxb@b.it",NULL};
    const char* p2[] = {"xxe@e.it","xxb@b.it",NULL};
    testVC(n1,n2,e1,e2,p1,p2);
  }

  {
    printf("\nTest %d\n",++test);
    str n1 = "Eugenio";
    str n2 = "Eugenio";
    const char* e1[] = {"111b@b.it","222e@e.it",NULL};
    const char* e2[] = {"222e@e.it","111b@b.it",NULL};
    const char* p1[] = {"xxe@e.it","xxb@b.it",NULL};
    const char* p2[] = {"xxe@e.it","xxb@b.it",NULL};
    testVC(n1,n2,e1,e2,p1,p2);
  }

  {
    printf("\nTest %d\n",++test);
    str n1 = "Eugenio";
    str n2 = "Eugenio";
    const char* e1[] = {"111b@b.it","222e@e.it",NULL};
    const char* e2[] = {"222e@e.it","111b@b.it",NULL};
    const char* p1[] = {"111xxb@b.it","222xxe@e.it",NULL};
    const char* p2[] = {"222xxe@e.it","111xxb@b.it",NULL};
    testVC(n1,n2,e1,e2,p1,p2);
  }

  {
    printf("\nTest %d\n",++test);
    str n1 = "Eugenio";
    str n2 = "Eugenio";
    const char* e1[] = {"111b@b.it","222e@e.it",NULL};
    const char* e2[] = {"222e@e.it","111b@b.it",NULL};
    const char* p1[] = {"111xxb@b.it","222xxe@e.it","111xxb@b.it","111xxb@b.it",NULL};
    const char* p2[] = {"222xxe@e.it","111xxb@b.it",NULL};
    testVC(n1,n2,e1,e2,p1,p2);
  }

  {
    printf("\nTest %d\n",++test);
    str n1 = "Eugenio";
    str n2 = "Eugenio";
    const char* e1[] = {"111b@b.it","222e@e.it",NULL};
    const char* e2[] = {"222e@e.it","111b@b.it",NULL};
    const char* p1[] = {"111xxb@b.it","222xxe@e.it","111xxb@b.it","111xxb@b.it",NULL};
    const char* p2[] = {"222xxe@e.it","111xxb@b.it",NULL};
    testVC(n1,n2,e1,e2,p1,p2);
  }

  {
    printf("\nTest %d\n",++test);
    str n1 = "Eugenio";
    str n2 = "Eugenio";
    const char* e1[] = {"111b@b.it","222e@e.it",NULL};
    const char* e2[] = {"222e@e.it","111b@b.it",NULL};
    const char* p1[] = {"111xxb@b.it","222xxe@e.it","111xxb@b.it","333xxb@b.it",NULL};
    const char* p2[] = {"222xxe@e.it","111xxb@b.it",NULL};
    testVC(n1,n2,e1,e2,p1,p2);
  }

  {
    printf("\nTest %d\n",++test);
    str n1 = "Eugenio";
    str n2 = "Eugeni";
    const char* e1[] = {"111b@b.it","222e@e.it",NULL};
    const char* e2[] = {"111b@b.it","222e@e.it",NULL};
    const char* p1[] = {"222xxe@e.it","111xxb@b.it",NULL};
    const char* p2[] = {"222xxe@e.it","111xxb@b.it",NULL};
    testVC(n1,n2,e1,e2,p1,p2);
  }

}

