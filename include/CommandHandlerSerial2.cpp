#include <Arduino.h>
#include <LinkedList.h>
#ifndef _COMMAND_HANDLER_SERIAL2_
#define _COMMAND_HANDLER_SERIAL2_
class CMDSerial2
{
  private:
  LinkedList<String> prikazy;
  LinkedList<int> navratky, vratitcislo;
  LinkedList<bool> maparametr;
  LinkedList<int> parametry;
  char stopznak, paralevy, parapravy;
  public:
  CMDSerial2 (char stpznk, char prlv, char prprv)
  {
    stopznak = stpznk;
    paralevy = prlv;
    parapravy = prprv;
    prikazy = LinkedList<String>();
    navratky = LinkedList<int>();
    vratitcislo = LinkedList<int>();
    maparametr = LinkedList<bool>();
    parametry = LinkedList<int>();
    PridejPrikaz(9999, "ˇ", false);
    Serial2.setTimeout(1);
  };
  void Update ()
  {
    LinkedList<String> prijateprikazy = LinkedList<String>();
    for (uint8_t i = 0; (Serial2.available() > 0) && (i <= 64); i++)
    {
      prijateprikazy.add(Serial2.readStringUntil(stopznak));
    }
    for (int i = 0; i < prijateprikazy.size(); i++)
    { 
      int element = _NajdiPrikaz(prijateprikazy.get(i));
      if (element == 0) continue;
      vratitcislo.add(navratky.get(element));
      if (maparametr.get(element))
      {
        parametry.add(_NajdiParametr(prijateprikazy.get(i)));
      }
      else parametry.add(0);
    } 
    return;
  }
  bool Next (int &rtrn, int &para)
  {
    if (vratitcislo.size() == 0) return false;
    rtrn = vratitcislo.get(0);
    para = parametry.get(0);
    vratitcislo.remove(0);
    parametry.remove(0);
    return true;
  }
  bool PridejPrikaz (int rtrn, String cmnd, bool pr)
  {
    if (rtrn == 0) return false;
    for (int i = 0; i < navratky.size(); i++)
    {
      if (rtrn == navratky.get(i)) return false;
      if (cmnd == prikazy.get(i)) return false;
    }
    navratky.add(rtrn);
    prikazy.add(cmnd);
    maparametr.add(pr);
    return true;
  }
  bool SmazPrikaz (int rtrn)
  {
    for (int i = 0; i < navratky.size(); i++)
    {
      if (rtrn == navratky.get(i))
      {
        navratky.remove(i);
        prikazy.remove(i);
        maparametr.remove(i);
        return true;
      }
    }
    return false;
  }
  bool SmazPrikaz (String cmnd)
  {
    for (int i = 0; i < prikazy.size(); i++)
    {
      if (cmnd == prikazy.get(i))
      {
        navratky.remove(i);
        prikazy.remove(i);
        maparametr.remove(i);
        return true;
      }
    }
    return false;
  }
  private:
  int _NajdiParametr (String vstup)
  {
    int l, r;
    l = vstup.indexOf(paralevy);
    r = vstup.indexOf(parapravy);
    if (l > r) return 0;
    if ((l + 1) == r) return 0;
    l += 1;
    String cislo = vstup.substring(l, r);
    return cislo.toInt();
  }
  int _NajdiPrikaz (String vstup)
  {
    for (int i = 0; i < prikazy.size(); i++)
    {
      if (vstup.indexOf(prikazy.get(i)) != -1)
      {
        return i; 
      }
    }
    return 0;
  }
};
#endif