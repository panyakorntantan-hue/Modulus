#include "cad_adapter.h"
#include <cmath>
#include <sstream>
#include <iomanip>

// ObjectARX headers
#include "acdb.h"
#include "dbents.h"
#include "dbsymtb.h"
#include "acedads.h"
#include "acutmem.h"

namespace ModulusLite {

// ============================================================================
// HELPER: Convert std::string to ACHAR (wchar_t)
// ============================================================================

static ACHAR* stringToAchar(const std::string& str) {
    size_t len = str.size() + 1;
    ACHAR* result = new ACHAR[len];
    size_t converted = std::mbstowcs(result, str.c_str(), len);
    return result;
}

// ============================================================================
// CAD ADAPTER IMPLEMENTATION
// ============================================================================

bool CADAdapter::initialize() {
    if (!m_pDb) return false;
    return true;
}

bool CADAdapter::drawMachine(const Machine& machine) {
    if (!m_pDb) return false;
    
    // Get block table
    AcDbBlockTable* pBlkTable;
    if (acdbOpenObject(pBlkTable, m_pDb->blockTableId(), AcDb::kForRead) != Acad::eOk) {
        return false;
    }
    
    // Get model space record ID
    AcDbObjectId msId;
    pBlkTable->getAt(ACDB_MODEL_SPACE, msId);
    
    // Open model space for write
    AcDbBlockTableRecord* pMs;
    if (acdbOpenObject(pMs, msId, AcDb::kForWrite) != Acad::eOk) {
        pBlkTable->close();
        return false;
    }
    
    // Create block reference
    AcDbBlockReference* pRef = new AcDbBlockReference();
    
    // Set position
    AcGePoint3d pos(machine.position.x, machine.position.y, machine.position.z);
    pRef->setPosition(pos);
    
    // Set scale
    AcGeScale3d scale(1.0, 1.0, 1.0);
    pRef->setScaleFactors(scale);
    
    // Try to reference block from DWG
    if (!machine.modelPath.empty()) {
        AcDbObjectId blockId;
        ACHAR* blockNameW = stringToAchar(machine.modelPath);
        
        Acad::ErrorStatus es = pBlkTable->getAt(blockNameW, blockId);
        if (es == Acad::eOk) {
            pRef->setBlockTableRecord(blockId);
        }
        // Fallback: invisible reference if block not found
        
        delete[] blockNameW;
    }
    
    // Add to model space
    AcDbObjectId refId;
    Acad::ErrorStatus es = pMs->appendAcDbEntity(refId, pRef);
    
    pMs->close();
    pBlkTable->close();
    pRef->close();
    
    return (es == Acad::eOk);
}

bool CADAdapter::drawPipe(const Pipe& pipe) {
    if (!m_pDb || pipe.path.empty()) return false;
    
    // Get block table and model space
    AcDbBlockTable* pBlkTable;
    if (acdbOpenObject(pBlkTable, m_pDb->blockTableId(), AcDb::kForRead) != Acad::eOk) {
        return false;
    }
    
    AcDbObjectId msId;
    pBlkTable->getAt(ACDB_MODEL_SPACE, msId);
    
    AcDbBlockTableRecord* pMs;
    if (acdbOpenObject(pMs, msId, AcDb::kForWrite) != Acad::eOk) {
        pBlkTable->close();
        return false;
    }
    
    // Create 3D polyline
    AcGePoint3dArray vertices;
    for (const auto& pt : pipe.path) {
        vertices.append(AcGePoint3d(pt.x, pt.y, pt.z));
    }
    
    AcDb3dPolyline* pPoly = new AcDb3dPolyline(AcDb::k3dSimplePoly, vertices);
    
    // Set color by material
    colorByMaterial(pPoly, pipe.material);
    
    // Set lineweight by diameter
    setLineweightByDiameter(pPoly, pipe.diameter);
    
    // Add to model space
    AcDbObjectId polyId;
    Acad::ErrorStatus es = pMs->appendAcDbEntity(polyId, pPoly);
    
    pMs->close();
    pBlkTable->close();
    pPoly->close();
    
    return (es == Acad::eOk);
}

bool CADAdapter::drawFlowArrow(const Point3D& position, const Point3D& direction) {
    if (!m_pDb) return false;
    
    // Normalize direction
    Point3D dir = direction.normalize();
    
    // Get model space
    AcDbBlockTable* pBlkTable;
    if (acdbOpenObject(pBlkTable, m_pDb->blockTableId(), AcDb::kForRead) != Acad::eOk) {
        return false;
    }
    
    AcDbObjectId msId;
    pBlkTable->getAt(ACDB_MODEL_SPACE, msId);
    
    AcDbBlockTableRecord* pMs;
    if (acdbOpenObject(pMs, msId, AcDb::kForWrite) != Acad::eOk) {
        pBlkTable->close();
        return false;
    }
    
    // Create arrow block reference
    AcDbBlockReference* pArrow = new AcDbBlockReference();
    
    AcGePoint3d arrowPos(position.x, position.y, position.z);
    pArrow->setPosition(arrowPos);
    
    // Rotate arrow to face direction
    double angle = std::atan2(dir.y, dir.x);
    pArrow->setRotation(angle);
    
    AcDbObjectId arrowId;
    Acad::ErrorStatus es = pMs->appendAcDbEntity(arrowId, pArrow);
    
    pMs->close();
    pBlkTable->close();
    
    return (es == Acad::eOk);
}

bool CADAdapter::drawPipeTag(const Pipe& pipe) {
    if (!m_pDb || pipe.path.empty()) return false;
    
    // Find midpoint of path
    double totalDist = 0.0;
    for (size_t i = 0; i + 1 < pipe.path.size(); ++i) {
        totalDist += pipe.path[i].distance(pipe.path[i+1]);
    }
    
    double targetDist = totalDist / 2.0;
    double currentDist = 0.0;
    
    Point3D midpoint = pipe.path[0];
    for (size_t i = 0; i + 1 < pipe.path.size(); ++i) {
        double segLen = pipe.path[i].distance(pipe.path[i+1]);
        if (currentDist + segLen >= targetDist) {
            double t = (segLen > 0) ? (targetDist - currentDist) / segLen : 0.0;
            midpoint = pipe.path[i] + (pipe.path[i+1] - pipe.path[i]) * t;
            break;
        }
        currentDist += segLen;
    }
    
    // Get model space
    AcDbBlockTable* pBlkTable;
    if (acdbOpenObject(pBlkTable, m_pDb->blockTableId(), AcDb::kForRead) != Acad::eOk) {
        return false;
    }
    
    AcDbObjectId msId;
    pBlkTable->getAt(ACDB_MODEL_SPACE, msId);
    
    AcDbBlockTableRecord* pMs;
    if (acdbOpenObject(pMs, msId, AcDb::kForWrite) != Acad::eOk) {
        pBlkTable->close();
        return false;
    }
    
    // Create MText for tag
    AcDbMText* pMText = new AcDbMText();
    
    AcGePoint3d tagPos(midpoint.x, midpoint.y, midpoint.z + 0.3);
    pMText->setLocation(tagPos);
    pMText->setTextHeight(0.25);
    
    // Convert tag to ACHAR
    ACHAR* tagW = stringToAchar(pipe.tag);
    pMText->setContents(tagW);
    delete[] tagW;
    
    AcDbObjectId textId;
    Acad::ErrorStatus es = pMs->appendAcDbEntity(textId, pMText);
    
    pMs->close();
    pBlkTable->close();
    pMText->close();
    
    return (es == Acad::eOk);
}

bool CADAdapter::drawMachineLabel(const Machine& machine) {
    if (!m_pDb) return false;
    
    // Create label string
    std::ostringstream label;
    label << machine.name << " (" << machine.id << ")";
    
    // Get model space
    AcDbBlockTable* pBlkTable;
    if (acdbOpenObject(pBlkTable, m_pDb->blockTableId(), AcDb::kForRead) != Acad::eOk) {
        return false;
    }
    
    AcDbObjectId msId;
    pBlkTable->getAt(ACDB_MODEL_SPACE, msId);
    
    AcDbBlockTableRecord* pMs;
    if (acdbOpenObject(pMs, msId, AcDb::kForWrite) != Acad::eOk) {
        pBlkTable->close();
        return false;
    }
    
    // Create MText label above machine
    AcDbMText* pLabel = new AcDbMText();
    
    Point3D labelPos = machine.position + Point3D(0, 0, machine.size.z / 2 + 0.5);
    AcGePoint3d pos(labelPos.x, labelPos.y, labelPos.z);
    pLabel->setLocation(pos);
    pLabel->setTextHeight(0.3);
    
    // Convert to ACHAR
    ACHAR* labelW = stringToAchar(label.str());
    pLabel->setContents(labelW);
    delete[] labelW;
    
    AcDbObjectId labelId;
    Acad::ErrorStatus es = pMs->appendAcDbEntity(labelId, pLabel);
    
    pMs->close();
    pBlkTable->close();
    pLabel->close();
    
    return (es == Acad::eOk);
}

bool CADAdapter::drawLegend() {
    if (!m_pDb) return false;
    
    // Get model space
    AcDbBlockTable* pBlkTable;
    if (acdbOpenObject(pBlkTable, m_pDb->blockTableId(), AcDb::kForRead) != Acad::eOk) {
        return false;
    }
    
    AcDbObjectId msId;
    pBlkTable->getAt(ACDB_MODEL_SPACE, msId);
    
    AcDbBlockTableRecord* pMs;
    if (acdbOpenObject(pMs, msId, AcDb::kForWrite) != Acad::eOk) {
        pBlkTable->close();
        return false;
    }
    
    // Create legend text
    std::string legendText = 
        "P&ID TAG FORMAT: [Line]-[Size]-[Material]-[Service]-[Spec]\n"
        "\nMaterial Codes: L=Liquid, G=Gas, S=Solid, SL=Sludge\n"
        "Service Codes: W=Water, AIR=Air, SLURRY=Slurry, CHEM=Chemical\n"
        "Spec Codes: CS150, SS316, PVC, HDPE\n"
        "Flow Direction: Indicated by arrows\n"
        "\nColor Legend:\n"
        "Blue=Liquid, Yellow=Gas, Brown=Solid, Gray=Sludge";
    
    // Create MText legend
    AcDbMText* pLegend = new AcDbMText();
    
    AcGePoint3d legendPos(1.0, 1.0, 0.0);
    pLegend->setLocation(legendPos);
    pLegend->setTextHeight(0.2);
    
    // Convert to ACHAR
    ACHAR* legendW = stringToAchar(legendText);
    pLegend->setContents(legendW);
    delete[] legendW;
    
    AcDbObjectId legendId;
    Acad::ErrorStatus es = pMs->appendAcDbEntity(legendId, pLegend);
    
    pMs->close();
    pBlkTable->close();
    pLegend->close();
    
    return (es == Acad::eOk);
}

void CADAdapter::colorByMaterial(AcDbEntity* pEnt, MaterialType material) {
    if (!pEnt) return;
    
    int colorIndex = getColorIndex(material);
    AcCmColor color;
    color.setColorIndex(colorIndex);
    pEnt->setColor(color);
}

int CADAdapter::getColorIndex(MaterialType material) {
    switch (material) {
        case MaterialType::Liquid: return 5;  // Cyan/Blue
        case MaterialType::Gas:    return 2;  // Yellow
        case MaterialType::Solid:  return 30; // Brown
        case MaterialType::Sludge: return 8;  // Gray
        default: return 256;
    }
}

void CADAdapter::setLineweightByDiameter(AcDbEntity* pEnt, double diameter) {
    if (!pEnt) return;
    
    // Map diameter to lineweight
    AcDb::LineWeight lw;
    if (diameter < 0.1) {
        lw = AcDb::kLnWt013;
    } else if (diameter < 0.2) {
        lw = AcDb::kLnWt025;
    } else if (diameter < 0.3) {
        lw = AcDb::kLnWt035;
    } else {
        lw = AcDb::kLnWt050;
    }
    
    pEnt->setLineWeight(lw);
}

// ============================================================================
// PIPELINE IMPLEMENTATION
// ============================================================================

bool Pipeline::execute(std::vector<Machine>& machines, std::vector<Pipe>& pipes) {
    // Step 1: Layout
    if (!executeLayout(machines)) {
        return false;
    }
    
    // Step 2: Route pipes
    if (!executeRouting(machines, pipes)) {
        return false;
    }
    
    // Step 3: Draw to CAD
    if (!executeDrawing(machines, pipes)) {
        return false;
    }
    
    return true;
}

bool Pipeline::executeLayout(std::vector<Machine>& machines) {
    if (machines.empty()) return false;
    
    FDGLayout::layout(machines);
    return true;
}

bool Pipeline::executeRouting(std::vector<Machine>& machines, std::vector<Pipe>& pipes) {
    for (auto& pipe : pipes) {
        // Find machines
        Machine* pFrom = nullptr;
        Machine* pTo = nullptr;
        
        for (auto& m : machines) {
            if (m.id == pipe.fromMachineId) pFrom = &m;
            if (m.id == pipe.toMachineId) pTo = &m;
        }
        
        if (!pFrom || !pTo) continue;
        
        // Get port positions
        Point3D start = pFrom->ports[std::min(pipe.fromPortIndex, 
                                              (int)pFrom->ports.size() - 1)];
        Point3D goal = pTo->ports[std::min(pipe.toPortIndex, 
                                           (int)pTo->ports.size() - 1)];
        
        // Route using A*
        auto result = ManhattanRouter::route(start, goal, machines, pipes);
        if (!result.success) {
            // Fallback: straight line
            result.path.clear();
            result.path.push_back(start);
            result.path.push_back(goal);
        }
        
        pipe.path = result.path;
        
        // Compute properties
        PipeProperties::computeLength(pipe);
        PipeProperties::computeCost(pipe);
        PipeProperties::generateTag(pipe);
    }
    
    return true;
}

bool Pipeline::executeDrawing(const std::vector<Machine>& machines, 
                              const std::vector<Pipe>& pipes) {
    // Draw machines
    for (const auto& machine : machines) {
        m_adapter.drawMachine(machine);
        m_adapter.drawMachineLabel(machine);
    }
    
    // Draw pipes
    for (const auto& pipe : pipes) {
        m_adapter.drawPipe(pipe);
        
        // Add flow arrows every ~2m
        if (pipe.path.size() > 1) {
            double totalLen = 0.0;
            for (size_t i = 0; i + 1 < pipe.path.size(); ++i) {
                totalLen += pipe.path[i].distance(pipe.path[i+1]);
            }
            
            double arrowSpacing = 2.0;
            double currentLen = arrowSpacing;
            double accum = 0.0;
            
            for (size_t i = 0; i + 1 < pipe.path.size(); ++i) {
                double segLen = pipe.path[i].distance(pipe.path[i+1]);
                while (accum + segLen >= currentLen) {
                    double t = (segLen > 0) ? (currentLen - accum) / segLen : 0.0;
                    Point3D arrowPos = pipe.path[i] + (pipe.path[i+1] - pipe.path[i]) * t;
                    Point3D arrowDir = (pipe.path[i+1] - pipe.path[i]).normalize();
                    m_adapter.drawFlowArrow(arrowPos, arrowDir);
                    currentLen += arrowSpacing;
                }
                accum += segLen;
            }
        }
        
        m_adapter.drawPipeTag(pipe);
    }
    
    // Draw legend
    m_adapter.drawLegend();
    
    return true;
}

}  // namespace ModulusLite