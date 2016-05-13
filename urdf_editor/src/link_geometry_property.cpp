
#include <qttreepropertybrowser.h>
#include <qtvariantproperty.h>

#include <urdf_editor/link_geometry_property.h>
#include <urdf_editor/common.h>
#include <urdf_editor/qt_file_browser.h>

#include <urdf_model/link.h>


#include <QFileInfo>
#include <QFileDialog>
#include "codecs/step_to_stl.h"

namespace
{

  bool isStepFile(const QString& file)
  {
    QFileInfo info (file);

    if (!info.exists() || !info.isFile())
      return false;

    QString ext = info.suffix().toLower();

    if (ext.isEmpty())
      return false;

    if (ext == QString("step") || ext == QString("stp"))
      return true;
    else
      return false;
  }

  bool stepToStl(const QString& step_file, QString& stl_file)
  {
    if (!isStepFile(step_file))
    {
      return false;
    }


    QString out_file = QFileDialog::getSaveFileName(NULL, "STEP will be converted to STL; Please choose the name for the STL file");
    std::string step = step_file.toStdString();
    std::string stl = out_file.toStdString();

    if (!urdf_editor::codecs::convertStepToStl(step, stl))
    {
      return false;
    }

    QFileInfo stl_info (QString::fromStdString(stl));

    if (!stl_info.exists() || !stl_info.isFile())
    {
      return false;
    }

    stl_file = QString("file://") + stl_info.absoluteFilePath();

    return true;
  }

}

namespace urdf_editor
{
  LinkGeometryProperty::LinkGeometryProperty(urdf::GeometrySharedPtr geometry): geometry_(geometry), manager_(new FileBrowserVariantManager()), factory_(new FileBrowserVariantFactory())
  {
    loading_ = true;
    QtVariantProperty *item;
    // QtVariantProperty *sub_item;

    QObject::connect(manager_, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
              this, SLOT(onValueChanged(QtProperty *, const QVariant &)));

    //{SPHERE, BOX, CYLINDER, MESH}
    top_item_ = manager_->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Geometry"));
    item = manager_->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Type"));
    item->setAttribute(Common::attributeStr(EnumNames), QStringList() << "SPHERE" << "BOX" << "CYLINDER" << "MESH");
    top_item_->addSubProperty(item);

    createProperties(geometry_->type);

    loading_ = false;
  }

  LinkGeometryProperty::~LinkGeometryProperty()
  {
    delete manager_;
    delete factory_;
  }

  void LinkGeometryProperty::loadData()
  {
    loading_ = true;
    QtVariantProperty *item;
    QString name;
    QList<QtProperty *> sub_items = top_item_->subProperties();
    for (int i = 0; i < sub_items.length(); ++i)
    {
      item = static_cast<QtVariantProperty *>(sub_items[i]);
      name = item->propertyName();

      if (name == "Type")
      {
        item->setValue(geometry_->type);
      }
      else
      {
        if (geometry_->type == urdf::Geometry::BOX)
        {
          boost::shared_ptr<urdf::Box> box = boost::static_pointer_cast<urdf::Box>(geometry_);
          if (name == "Length X (m)")
            item->setValue(box->dim.x);
          else if (name == "Length Y (m)")
            item->setValue(box->dim.y);
          else if (name == "Length Z (m)")
            item->setValue(box->dim.z);
        }
        else if (geometry_->type == urdf::Geometry::CYLINDER)
        {
          boost::shared_ptr<urdf::Cylinder> cylinder = boost::static_pointer_cast<urdf::Cylinder>(geometry_);
          if (name == "Radius (m)")
            item->setValue(cylinder->radius);
          else if (name == "Length (m)")
            item->setValue(cylinder->length);
        }
        else if (geometry_->type == urdf::Geometry::SPHERE)
        {
          boost::shared_ptr<urdf::Sphere> sphere = boost::static_pointer_cast<urdf::Sphere>(geometry_);
          if (name == "Radius (m)")
            item->setValue(sphere->radius);
        }
        else if (geometry_->type == urdf::Geometry::MESH)
        {
          boost::shared_ptr<urdf::Mesh> mesh = boost::static_pointer_cast<urdf::Mesh>(geometry_);
          if (name == "File Name")
            item->setValue(QString::fromStdString(mesh->filename));

          if (name == "Scale")
          {
            //Remaining sub-properties are for Mesh
            QList<QtProperty *> sub_items =  top_item_ ->subProperties()[2]->subProperties();
           
            for (int i = 0; i < sub_items.length(); ++i)  //load scale values
            {
              item = static_cast<QtVariantProperty *>(sub_items[i]);
              name = item->propertyName();
              if (name == "X")
                item->setValue(mesh->scale.x);
              else if (name == "Y")
                item->setValue(mesh->scale.y);
              else if (name == "Z")
                item->setValue(mesh->scale.z);
            }
          }
        }
      }
    }
    loading_ = false;
  }

  void LinkGeometryProperty::loadFactoryForManager(QtTreePropertyBrowserSharedPtr& property_editor)
  {
    property_editor->setFactoryForManager(manager_, factory_);
  }

  void LinkGeometryProperty::onValueChanged(QtProperty *property, const QVariant &val)
  {
    if (loading_)
      return;

    QString name = property->propertyName();
    if (name == "Type")
    {
      loading_ = true;
      
      // remove all the subproperties
      QList<QtProperty *> sub_items = top_item_->subProperties();
      for (int i = 0; i < sub_items.length(); ++i) 
        if (sub_items[i]->propertyName() != "Type")
          top_item_->removeSubProperty(sub_items[i]);

      createProperties(val.toInt());

      loading_ = false;
    }
    else
    {
      if (geometry_->type == urdf::Geometry::BOX)
      {
        boost::shared_ptr<urdf::Box> box = boost::static_pointer_cast<urdf::Box>(geometry_);
        if (name == "Length X (m)")
          box->dim.x = val.toDouble();
        else if (name == "Length Y (m)")
          box->dim.y = val.toDouble();
        else if (name == "Length Z (m)")
          box->dim.z = val.toDouble();
      }
      else if (geometry_->type == urdf::Geometry::CYLINDER)
      {
        boost::shared_ptr<urdf::Cylinder> cylinder = boost::static_pointer_cast<urdf::Cylinder>(geometry_);
        if (name == "Radius (m)")
          cylinder->radius = val.toDouble();
        else if (name == "Length (m)")
          cylinder->length = val.toDouble();
      }
      else if (geometry_->type == urdf::Geometry::SPHERE)
      {
        boost::shared_ptr<urdf::Sphere> sphere = boost::static_pointer_cast<urdf::Sphere>(geometry_);
        if (name == "Radius (m)")
          sphere->radius = val.toDouble();
      }
      else if (geometry_->type == urdf::Geometry::MESH)
      {
        boost::shared_ptr<urdf::Mesh> mesh = boost::static_pointer_cast<urdf::Mesh>(geometry_);
        if (name == "File Name")
        {
          // TODO: convert absolute URI to a ROS pkg relative one
          QString path = val.toString();

          if (isStepFile(path))
          {
            QString new_stl_file;
            if (stepToStl(path, new_stl_file))
            {
              path = new_stl_file;
            }
          }

          if (!path.startsWith("package://") && !path.startsWith("file://"))
          {
            path = QString("file://%1").arg(path);
          }

          QtVariantProperty* item = static_cast<QtVariantProperty *>(property);
          item->setValue(path);
          mesh->filename = path.toStdString();
        }
        else if (name == "X")
          mesh->scale.x = val.toDouble();
        else if (name == "Y")
          mesh->scale.y = val.toDouble();
        else if (name == "Z")
          mesh->scale.z = val.toDouble();
      }
    }

    emit LinkGeometryProperty::valueChanged(property, val);
  }

  void LinkGeometryProperty::createProperties(int type)
  {
    QtVariantProperty *item;
    QtVariantProperty *sub_item;
    
    switch (type) //{SPHERE, BOX, CYLINDER, MESH}
    {
    case 0:
      {
      item = manager_->addProperty(QVariant::Double, tr("Radius (m)"));
      top_item_->addSubProperty(item);
      break;
      }
    case 1:
      {        
      item = manager_->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Size"));
      sub_item = manager_->addProperty(QVariant::Double, tr("Length X (m)"));
      item->addSubProperty(sub_item);

      sub_item = manager_->addProperty(QVariant::Double, tr("Length Y (m)"));
      item->addSubProperty(sub_item);

      sub_item = manager_->addProperty(QVariant::Double, tr("Length Z (m)"));
      item->addSubProperty(sub_item);
      top_item_->addSubProperty(item);
      break;
      }
    case 2:
      {
      item = manager_->addProperty(QVariant::Double, tr("Radius (m)"));
      top_item_->addSubProperty(item);

      item = manager_->addProperty(QVariant::Double, tr("Length (m)"));
      top_item_->addSubProperty(item);
      break;
      }
    case 3:
      {
      item = manager_->addProperty(FileBrowserVariantManager::filePathTypeId(), tr("File Name"));
      top_item_->addSubProperty(item);

      item = manager_->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Scale"));
      sub_item = manager_->addProperty(QVariant::Double, tr("X"));
      item->addSubProperty(sub_item);
      sub_item = manager_->addProperty(QVariant::Double, tr("Y"));
      item->addSubProperty(sub_item);
      sub_item = manager_->addProperty(QVariant::Double, tr("Z"));
      item->addSubProperty(sub_item);
      top_item_->addSubProperty(item);
      break;
      }
    }
  }
}
