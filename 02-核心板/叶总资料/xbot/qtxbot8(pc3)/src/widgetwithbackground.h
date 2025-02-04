#ifndef WIDGETWITHBACKGROUND_H
#define WIDGETWITHBACKGROUND_H

#include <QWidget>

class QPixmap;

class WidgetWithBackground : public QWidget
{
	Q_OBJECT
  public:
     /** Konstruktor */
     WidgetWithBackground(QWidget * parent = 0);

     /** Destruktor - usuwa bitmapk?  */
     ~WidgetWithBackground ();

     /**
      * Operacja odmalowywania kontrolki  -
      * wkleja bitmap?z t�em na kontrolk?-
      * generuj�� j?wcze�niej je�eli zajdzie taka potrzeba.
      */
     void  drawBackground ();

     /** Odmalowywuje kontrolk?bezwarunkowo odmalowywuj�c t�o. */
     void  updateWithBackground ();
    
     /** 
     * Zawraca informacje czy kontrolka zosta�a zmodyfikowana
     * Ta informacja jest ustawiana gdy bitmapka t�a si?zmienia 
     * to znaczy zmienia si?rozmiar komponentu lub istnieje 
     * potrzeba przemalowania t�a. 
     */ 
     bool doRepaintBackground(); 
     
  protected:
    /** Wywo�uje paintBackground - odmalowywuj�c t�o na nowo */
    void repaintBackground();

    /**
     * Odmalowywuje t�o kontrolki
     * @param painter urz�dzenie na ktr�ym mamy malowa?
     * @param background to t�o kontrolki
     */
    virtual void  paintBackground (QPainter & painer) = 0;
    
    

  protected:
     /** Bufor na t�o. */
     QPixmap * m_pixmap;
     /**
     * Ustawia t?zmienn?po zmianie w�a�ciwo�ci
     */
     bool m_modified;
};
#endif //WIDGETWITHBACKGROUND_H

