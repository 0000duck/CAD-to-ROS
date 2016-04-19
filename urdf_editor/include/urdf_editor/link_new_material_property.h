#ifndef __LINK_NEW_MATERIAL_PROPERTY_H__
#define __LINK_NEW_MATERIAL_PROPERTY_H__

#include <QtCore>

#include <urdf_editor/qt_types.h>
#include <urdf_editor/urdf_types_ext.h>


namespace urdf_editor
{
  class LinkNewMaterialProperty : public QObject
  {
    Q_OBJECT
  public:
    LinkNewMaterialProperty(urdf::MaterialSharedPtr material);
    ~LinkNewMaterialProperty();

    void loadFactoryForManager(QtTreePropertyBrowserSharedPtr& property_editor);

    void loadData();

    QtProperty *getTopItem() { return top_item_; }

  private slots:
    void onValueChanged(QtProperty *property, const QVariant &val);

  signals:
    void valueChanged(QtProperty *property, const QVariant &val);

  private:
    urdf::MaterialSharedPtr material_;
    QtVariantPropertyManager *manager_;
    QtVariantEditorFactory *factory_;
    QtProperty *top_item_;
    bool loading_;
  };
}

#endif // __LINK_NEW_MATERIAL_PROPERTY_H__
