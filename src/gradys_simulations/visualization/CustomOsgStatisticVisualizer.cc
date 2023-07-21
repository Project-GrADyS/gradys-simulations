////
//// This program is free software: you can redistribute it and/or modify
//// it under the terms of the GNU Lesser General Public License as published by
//// the Free Software Foundation, either version 3 of the License, or
//// (at your option) any later version.
////
//// This program is distributed in the hope that it will be useful,
//// but WITHOUT ANY WARRANTY; without even the implied warranty of
//// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//// GNU Lesser General Public License for more details.
////
//// You should have received a copy of the GNU Lesser General Public License
//// along with this program.  If not, see http://www.gnu.org/licenses/.
////
//
//#include "CustomOsgStatisticVisualizer.h"
//#include "inet/common/ModuleAccess.h"
//#include "inet/common/OsgUtils.h"
//
//namespace gradys_simulations {
//
//using namespace inet;
//
//Define_Module(CustomOsgStatisticVisualizer);
//
//#ifdef WITH_OSG
//
//visualizer::StatisticVisualizerBase::StatisticVisualization *CustomOsgStatisticVisualizer::createStatisticVisualization(cComponent *source, simsignal_t signal) {
//    auto label = new osgText::Text();
//    label->setCharacterSize(18);
//    label->setBoundingBoxColor(inet::osg::Vec4(backgroundColor.red / 255.0, backgroundColor.green / 255.0, backgroundColor.blue / 255.0, opacity));
//    label->setColor(inet::osg::Vec4(textColor.red / 255.0, textColor.green / 255.0, textColor.blue / 255.0, 0.5));
//    label->setAlignment(osgText::Text::CENTER_BOTTOM);
//    label->setText("");
//    label->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
//    label->setPosition(inet::osg::Vec3(0.0, 0.0, 0.0));
//    auto geode = new inet::osg::Geode();
//    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, inet::osg::StateAttribute::OFF | inet::osg::StateAttribute::OVERRIDE);
//    geode->addDrawable(label);
//    auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
//    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
//    if (networkNodeVisualization == nullptr)
//      throw cRuntimeError("Cannot create statistic visualization for '%s', because network node visualization is not found for '%s'", source->getFullPath().c_str(), networkNode->getFullPath().c_str());
//    return new StatisticOsgVisualization(networkNodeVisualization, geode, source->getId(), signal, getUnit(source));
//}
//
//#endif
//} /* namespace gradys_simulations */
