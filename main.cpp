
#include <string>
#include <unordered_map>
#include "pugixml.hpp"

#include "SQLParser.h"
#include "util/sqlhelper.h"

bool IsUnusableWhere(hsql::Expr* wherestmt)
{
    if (wherestmt->type == hsql::kExprOperator)
    {
        switch (wherestmt->opType)
        {
        case hsql::kOpEquals:
        case hsql::kOpNotEquals:
        case hsql::kOpLess:
        case hsql::kOpLessEq:
        case hsql::kOpGreater:
        case hsql::kOpGreaterEq:
            if (wherestmt->expr->type == hsql::kExprColumnRef)
            {
                if (strcmp(wherestmt->expr->name, "scale_denominator") == 0)
                    return true;
                if (strcmp(wherestmt->expr->name, "way_area") == 0)
                    return true;
                if (strcmp(wherestmt->expr->name, "pixel_width") == 0)
                    return true;
                if (strcmp(wherestmt->expr->name, "pixel_height") == 0)
                    return true;
            }
            if (wherestmt->expr2->type == hsql::kExprColumnRef)
            {
                if (strcmp(wherestmt->expr2->name, "scale_denominator") == 0)
                    return true;
                if (strcmp(wherestmt->expr2->name, "way_area") == 0)
                    return true;
                if (strcmp(wherestmt->expr2->name, "pixel_width") == 0)
                    return true;
                if (strcmp(wherestmt->expr2->name, "pixel_height") == 0)
                    return true;
            }
            break;
        default:return false;
        }
    }
    return false;
}

enum WhereOptions
{
    WO_AttributeNameLaundering = 0x1,
    WO_QuoteLiterals = 0x2,
};
std::string ParseWhere(hsql::Expr* wherestmt, char localRef, char options = 0)
{
    std::string ret = "";
    if (wherestmt->type == hsql::kExprOperator)
    {
        //There's PostGIS scale data we can't and don't want to use
        if (IsUnusableWhere(wherestmt))
        {
            //Dummy condition
            //Simply easier to inject an 'always true' comparison
            //than to risk bugs from modifying the rest of the query
            ret += "2 > 1";
        }
        else if (wherestmt->opType == hsql::kOpConcat)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " || ";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
            return ret;
        }
        else if (wherestmt->opType == hsql::kOpCase)
        {
            ret += "(CASE";
            if (localRef == 1)
                localRef = 0;
            for (auto l : *wherestmt->exprList)
            {
                ret += " WHEN ";
                ret += ParseWhere(l, localRef, options);
            }
            if (wherestmt->expr2)
            {
                ret += " ELSE ";
                ret += ParseWhere(wherestmt->expr2, localRef, options);
            }
            ret += " END)";
        }
        else if (wherestmt->opType == hsql::kOpCaseListElement)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " THEN ";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
        }
        else if (wherestmt->opType == hsql::kOpIn)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " IN (";
            bool first = true;
            for (auto l : *wherestmt->exprList)
            {
                if (!first)
                    ret += ",";
                ret += ParseWhere(l, localRef, options);
                first = false;
            }
            ret += ")";
        }
        else if (wherestmt->opType == hsql::kOpEquals)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " = ";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
        }
        else if (wherestmt->opType == hsql::kOpMinus)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " - ";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
        }
        else if (wherestmt->opType == hsql::kOpPlus)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " + ";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
        }
        else if (wherestmt->opType == hsql::kOpSlash)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " / ";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
        }
        else if (wherestmt->opType == hsql::kOpAnd)
        {
            ret += "(";
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += ") AND (";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
            ret += ")";
        }
        else if (wherestmt->opType == hsql::kOpOr)
        {
            ret += "(";
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += ") OR (";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
            ret += ")";
        }
        else if (wherestmt->opType == hsql::kOpIsNull)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " IS NULL";
        }
        else if (wherestmt->opType == hsql::kOpLess)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options | WO_QuoteLiterals);
            ret += " < ";
            ret += ParseWhere(wherestmt->expr2, localRef, options | WO_QuoteLiterals);
        }
        else if (wherestmt->opType == hsql::kOpGreater)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options | WO_QuoteLiterals);
            ret += " > ";
            ret += ParseWhere(wherestmt->expr2, localRef, options | WO_QuoteLiterals);
        }
        else if (wherestmt->opType == hsql::kOpLessEq)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options | WO_QuoteLiterals);
            ret += " <= ";
            ret += ParseWhere(wherestmt->expr2, localRef, options | WO_QuoteLiterals);
        }
        else if (wherestmt->opType == hsql::kOpGreaterEq)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options | WO_QuoteLiterals);
            ret += " >= ";
            ret += ParseWhere(wherestmt->expr2, localRef, options | WO_QuoteLiterals);
        }
        else if (wherestmt->opType == hsql::kOpNotEquals)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options | WO_QuoteLiterals);
            ret += " != ";
            ret += ParseWhere(wherestmt->expr2, localRef, options | WO_QuoteLiterals);
        }
        else if (wherestmt->opType == hsql::kOpAsterisk)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += " * ";
            ret += ParseWhere(wherestmt->expr2, localRef, options);
        }
        else if (wherestmt->opType == hsql::kOpNot)
        {
            ret += " NOT (";
            ret += ParseWhere(wherestmt->expr, localRef, options);
            ret += ")";
        }
        else if (wherestmt->opType == hsql::kOpPosix)
        {
            //All of the Posix commands in the basic OSM style were determining numbers
            ret += " CAST(" + ParseWhere(wherestmt->expr, localRef, options) + " AS numeric) IS NOT NULL ";
        }
        else if (wherestmt->opType == hsql::kOpLike)
        {
            ret += ParseWhere(wherestmt->expr, localRef, options | WO_QuoteLiterals);
            ret += " LIKE ";
            ret += ParseWhere(wherestmt->expr2, localRef, options | WO_QuoteLiterals);
        }
        else
        {
            if (wherestmt->expr)
                ret += ParseWhere(wherestmt->expr, localRef, options);
            if (wherestmt->expr2)
                ret += ParseWhere(wherestmt->expr2, localRef, options);
        }
    }
    else if (wherestmt->type == hsql::kExprLiteralString)
    {
        ret += "\'";
        ret += wherestmt->name;
        ret += "\'";
    }
    else if (wherestmt->type == hsql::kExprColumnRef)
    {
        if (!localRef) ret += "[";
        ret += wherestmt->name;
        if (!localRef) ret += "]";
        if ((options & WO_AttributeNameLaundering) != 0)
        {
            while (true)
            {
                auto pos = ret.find(':');
                if (pos == std::string::npos)
                    break;
                ret.replace(pos, 1, "_");
            }
        }
    }
    else if (wherestmt->type == hsql::kExprLiteralNull)
    {
        ret = "NULL";
    }
    else if (wherestmt->type == hsql::kExprFunctionRef)
    {
        ret += wherestmt->name;
        ret += "(";
        for (int i = 0; i < wherestmt->exprList->size(); ++i)
        {
            if (i > 0)
                ret += ",";
            ret += ParseWhere((*wherestmt->exprList)[i], localRef, options);
        }
        ret += ")";
    }
    else if (wherestmt->type == hsql::kExprLiteralInt)
    {
        if ((options & WO_QuoteLiterals) != 0) ret += "'";
        ret += std::to_string(wherestmt->ival);
        if ((options & WO_QuoteLiterals) != 0) ret += "'";
    }
    else if (wherestmt->type == hsql::kExprLiteralFloat)
    {
        if ((options & WO_QuoteLiterals) != 0) ret += "'";
        ret += std::to_string(wherestmt->fval);
        if ((options & WO_QuoteLiterals) != 0) ret += "'";
    }
    else
    {
        ret = "";
    }

    return ret;
}

struct ConfigData
{
    std::unordered_map<std::string, char> attribs;
    std::vector<std::string> computed;
    std::string whereStmt;
    bool need_osmId;
};

std::vector<std::string> ToConfigStr(const hsql::SQLStatement* statement, ConfigData& data)
{
    std::vector<std::string> ret;
    if (statement->isType(hsql::kStmtSelect)) {
        const auto* select = static_cast<const hsql::SelectStatement*>(statement);
        for (auto expr : *select->selectList)
        {
            if (expr->type == hsql::kExprColumnRef)
            {
                if (strcmp(expr->name, "way_pixels") != 0)
                {
                    data.attribs[expr->name] = 1;
                }
            }
            else if (!expr->alias || strcmp(expr->alias, "way_pixels") == 0 || strcmp(expr->alias, "way") == 0)
            {
                continue;
            }
            if (expr->type == hsql::kExprFunctionRef)
            {
                std::vector<std::string> partNames;
                data.computed.push_back(expr->alias);
                ret.push_back(std::string(expr->alias) + "_type=String");
                std::string selstmt = "SELECT ";
                selstmt += expr->name;
                selstmt += "(";

                char localRef = 1;
                if (!(select->fromTable && select->fromTable->select && select->fromTable->select->selectList))
                    localRef = 0;
                else 
                    if (strcmp(expr->name, "round") == 0)
                    localRef = 0;

                for (int i = 0; i < expr->exprList->size(); ++i)
                {
                    if (i > 0)
                        selstmt += ",";
                    std::string tmp = ParseWhere((*expr->exprList)[i], localRef);
                    partNames.push_back(tmp);
                    selstmt += tmp;
                }
                selstmt += ")";
                bool first = true;
                if (select->fromTable && select->fromTable->select && select->fromTable->select->selectList)
                for (auto wherestmt : *select->fromTable->select->selectList)
                {
                    if (wherestmt->alias)
                    {
                        for (int i = 0; i < partNames.size(); ++i)
                        {
                            if (strcmp(partNames[i].c_str(), wherestmt->alias) == 0)
                            {
                                if (first)
                                {
                                    first = 0;
                                    selstmt += " FROM (SELECT (";
                                }
                                else
                                    selstmt += ", (";
                                selstmt += ParseWhere(wherestmt, 0);
                                selstmt += ") AS ";
                                selstmt += wherestmt->alias;
                            }
                        }
                    }
                }
                if (!first)
                    selstmt += ")";
                ret.push_back(std::string(expr->alias) + "_sql=" + selstmt);
            }
            if (expr->type == hsql::kExprOperator && expr->alias)
            {
                data.computed.push_back(expr->alias);
                ret.push_back(std::string(expr->alias) + "_type=String");
                std::string selstmt = "SELECT ";

                char localRef = 2;
                if (!(select->fromTable && select->fromTable->select && select->fromTable->select->selectList))
                    localRef = 0;
                selstmt += ParseWhere(expr, localRef);

                bool first = true;
                if (select->fromTable && select->fromTable->select && select->fromTable->select->selectList)
                    for (auto wherestmt : *select->fromTable->select->selectList)
                    {
                        if (wherestmt->alias)
                        {
                            if (strcmp(expr->alias, wherestmt->alias) == 0)
                            {
                                if (first)
                                {
                                    first = 0;
                                    selstmt += " FROM (SELECT (";
                                }
                                else
                                    selstmt += ", (";
                                selstmt += ParseWhere(wherestmt, 0);
                                selstmt += ") AS ";
                                selstmt += wherestmt->alias;
                            }
                        }
                    }
                if (!first)
                    selstmt += ")";
                ret.push_back(std::string(expr->alias) + "_sql=" + selstmt);
            }
        }

        if (select->fromTable && select->fromTable->select && select->fromTable->select->whereClause)
        {
            std::string whereclause = ParseWhere(select->fromTable->select->whereClause, 2, WO_AttributeNameLaundering);
            data.whereStmt = whereclause;
            printf("%s\n", whereclause.c_str());
        }
        else if (select->whereClause)
        {
            std::string whereclause = ParseWhere(select->whereClause, 2, WO_AttributeNameLaundering);
            data.whereStmt = whereclause;
            printf("%s\n", whereclause.c_str());
        }
        else
            data.whereStmt = "";
    }
    else
    {
        printf("Wrong type?\n");
    }

    return ret;
}

void ToConfigFile(std::vector<std::string> features, ConfigData& data, char types, std::string name)
{
    std::string whereQuery = "#WHEREQUERY=" + data.whereStmt;

    std::vector<std::string> possibleAttribs = {
        "access","addr:housename","addr:housenumber","addr:interpolation","admin_level","aerialway","aeroway","amenity","area","barrier","bicycle","brand","bridge","boundary","building","capital","construction","covered","culvert","cutting","denomination","disused","ele","embankment","foot","generator:source","harbour","highway","historic","horse","intermittent","junction","layer","leisure","lock","man_made","military","motorcar","name","office","oneway","operator","place","population","power","power_source","public_transport","railway","ref","religion","route","service","shop","sport","surface","toll","tourism","tower:type","tracktype","tunnel","water","waterway","width","wood","wetland","landuse","natural"
    };
    std::string ignore = "", valids = "";
    for (auto& p : possibleAttribs)
    {
        if (data.attribs.find(p) != data.attribs.end())
        {
            if (!valids.empty())
                valids += ",";
            valids += p;
        }
        else
        {
            if (!ignore.empty())
                ignore += ",";
            ignore += p;
        }
    }
    std::string tofile;
    tofile += whereQuery + "\n";
    
    std::string finalValids = valids;
    while (true)
    {
        auto pos = finalValids.find(':');
        if (pos == std::string::npos)
            break;
        finalValids.replace(pos, 1, "_");
    }

    tofile += "#VALIDATTRIBUTES=" + finalValids;
    for (int i = 0; i < data.computed.size(); ++i)
    {
        if (i > 0 || valids.size() > 0)
            tofile += ",";
        tofile += data.computed[i];
    }
    if (data.need_osmId)
        tofile += ",osm_id";
    tofile += "\n";
    tofile += "#VALIDLAYERS=";
    for (int i = 0; i < 5; ++i)
    {
        bool doThisOne = (types & (1 << i)) != 0;
        if (doThisOne)
            switch (i)
            {
            case 0: tofile += "points "; break;
            case 1: tofile += "lines "; break;
            case 2: tofile += "multipolygons "; break;
            case 3: tofile += "multilinestrings "; break;
            case 4: tofile += "other_relations "; break;
            }
    }
    tofile += "\n";

    tofile += "closed_ways_are_polygons=aeroway,amenity,boundary,building,craft,geological,historic,landuse,leisure,military,natural,office,place,shop,sport,tourism,highway=platform,public_transport=platform\n";
    tofile += "attribute_name_laundering=yes\n";

    for (int i = 0; i < 5; ++i)
    {
        bool doThisone = (types & (1 << i)) != 0;

        switch (i)
        {
        case 0: tofile += "[points]\n"; break;
        case 1: tofile += "[lines]\n"; break;
        case 2: tofile += "[multipolygons]\n"; break;
        case 3: tofile += "[multilinestrings]\n"; break;
        case 4: tofile += "[other_relations]\n"; break;
        }
        if (doThisone)
        {
            tofile += "osm_id=" + std::string(data.need_osmId ? "yes" : "no") + "\nosm_version=no\nosm_timestamp=no\nosm_uid=no\nosm_user=no\nosm_changeset=no\n";
            tofile += "attributes=";
            if (valids.size() > 0 && ignore.size() > 0)
                tofile += valids + "," + ignore;
            else if (valids.size() > 0)
                tofile += valids;
            else
                tofile += ignore;

            tofile += "\n";
            tofile += "ignore=note,note:,source,source_ref,source:,attribution,comment,fixme,created_by,odbl,odbl:note,SK53_bulk:load,tiger:,NHD:,nhd:,gnis:,geobase:,accuracy:meters,sub_sea:type,waterway:type,KSJ2:,yh:,osak:,kms:,ngbe:,naptan:,CLC:,3dshapes:ggmodelk,AND_nosr_r,import,it:fvg:\n";
            if (data.computed.size() > 0)
            {
                tofile += "computed_attributes=";
                for (int c = 0; c < data.computed.size(); ++c)
                {
                    if (c > 0)
                        tofile += ",";
                    tofile += data.computed[c];
                }
                tofile += "\n";
                for (auto& f : features)
                {
                    tofile += f + "\n";
                }
            }
        }
        else
        {
            tofile += "osm_id=no\nosm_version=no\nosm_timestamp=no\nosm_uid=no\nosm_user=no\nosm_changeset=no\n";
            tofile += "attributes=\n";
            tofile += "ignore=note,note:,source,source_ref,source:,attribution,comment,fixme,created_by,odbl,odbl:note,SK53_bulk:load,tiger:,NHD:,nhd:,gnis:,geobase:,accuracy:meters,sub_sea:type,waterway:type,KSJ2:,yh:,osak:,kms:,ngbe:,naptan:,CLC:,3dshapes:ggmodelk,AND_nosr_r,import,it:fvg:,";

            if (valids.size() > 0 && ignore.size() > 0)
                tofile += valids + "," + ignore;
            else if (valids.size() > 0)
                tofile += valids;
            else
                tofile += ignore;
            tofile += "\n";
        }
    }

    FILE* f = 0;
    name += ".ini";
    std::string filePath = "Output\\ini\\" + name;
    fopen_s(&f, filePath.c_str(), "wb");
    fwrite(tofile.c_str(), 1, tofile.size(), f);
    fclose(f);
}

void EliminateFunction(std::string& query, std::string fn)
{
    auto start = query.find(fn);
    while (start != std::string::npos)
    {
        int i;
        int count = 0;
        int startSize = query.size();
        for (i = start; i < query.size(); ++i)
        {
            if (query[i] == '(')
                count++;
            if (query[i] == ')')
            {
                count--;
                if (count <= -1)
                {
                    if (count == -1) 
                    {
                        query.erase(query.begin() + start, query.begin() + i);
                    }
                    else//Something has gone wrong
                        return;
                }
            }
        }
        if (i == startSize)
            return;
        start = query.find(fn + " (");
    }
}
void EliminateComments(std::string& query)
{
    auto start = query.find("--");
    while (start != std::string::npos)
    {
        auto end = query.find("\n", start);
        query.erase(start, end-start);

        start = query.find("--");
    }
}

void HandleProjectXml(std::string path)
{
    pugi::xml_document doc;
    doc.load_file(path.c_str());

    for (pugi::xml_node xlayer = doc.first_child().child("Layer"); xlayer; xlayer = xlayer.next_sibling("Layer"))
    {
        std::string query = "";
        for (pugi::xml_node xparam = xlayer.child("Datasource").child("Parameter"); xparam; xparam = xparam.next_sibling("Parameter"))
        {
            if (strcmp(xparam.attribute("name").as_string(), "table") == 0)
            {
                query = xparam.child_value();
                break;
            }
        }
        if (query.empty())
            continue;
        char types = 0;
        if (strstr(query.c_str(), "planet_osm_line") != 0)
        {
            types = (1 << 1) | (1 << 3);
        }
        if (strstr(query.c_str(), "planet_osm_roads") != 0)
        {
            types = 0xF;
        }
        if (strstr(query.c_str(), "planet_osm_point") != 0)
        {
            types = (1 << 0);
        }
        if (strstr(query.c_str(), "planet_osm_polygon") != 0)
        {
            types = (1 << 2);
        }
        if (types == 0)
            types = (1 << 4);

        std::vector<std::pair<std::string, std::string>> replaceStrs = {
            {"!scale_denominator!","scale_denominator"},
            {"!pixel_width!::real","pixel_width"},
            {"!pixel_height!::real","pixel_height"},
            {"ST_PointOnSurface(way) as way","way"},
            {"AND way && !bbox!",""},
            {"!bbox!","bbox"},
            {"round(ascii(md5(osm_id::text)) / 55)","round(osm_id / 55)"},
            {"::text",""},
            {"::TEXT",""},
            {"::integer",""},
            {"::INTEGER",""},
            {"::NUMERIC",""},
            {"layer~E'^-?\\\\d+$' AND ",""},
            {"NULLS LAST",""},
        };

        EliminateFunction(query, "JOIN");
        EliminateComments(query);
        for (auto& r : replaceStrs)
        {

            for (auto find = query.find(r.first); find != std::string::npos; find = query.find(r.first))
            {
                query.replace(query.begin() + find, query.begin() + find + r.first.size(), r.second);
            }
        }
        if (false)
        for (auto find = query.find("ORDER BY"); find != std::string::npos; find = query.find("ORDER BY"))
        {
            size_t descPos = query.find("DESC", find);
            size_t ascPos = query.find("ASC", find);
            if (query.find("DESC", descPos+4) != std::string::npos)
                descPos = query.find("DESC", descPos+4);
            if (descPos == std::string::npos && ascPos == std::string::npos)
            {
                //Error case;
                break;
            }
            if (descPos == std::string::npos)
            {
                query.erase(query.begin() + find, query.begin() + ascPos + 3);
            }
            else if (ascPos == std::string::npos)
            {
                query.erase(query.begin() + find, query.begin() + descPos + 4);
            }
            else if (ascPos < descPos)
            {
                query.erase(query.begin() + find, query.begin() + descPos + 4);
            }
            else
            {
                query.erase(query.begin() + find, query.begin() + ascPos + 3);
            }
        }
        auto find = query.rfind("AS");
        query.erase(query.begin() + find, query.end());

        hsql::SQLParserResult result;
        hsql::SQLParser::parse(query, &result);

        if (!result.isValid() && strcmp(result.errorMsg(), "syntax error, unexpected end of file, expecting ')'") == 0)
        {
            query += ")";
            hsql::SQLParser::parse(query, &result);
        }
        if (result.isValid() && result.size() > 0) {
            const hsql::SQLStatement* statement = result.getStatement(0);
            ConfigData data = {};
            if (query.find("osm_id") != std::string::npos)
                data.need_osmId = 1;

            auto features = ToConfigStr(statement, data);
            ToConfigFile(features, data, types, xlayer.attribute("name").as_string());
        }
        else
        {
            printf("ERROR PARSING: %s\n", query.c_str());
        }
    }

}

int main(int argc, char* argv[])
{
    std::string projectXML = "project.xml";
    if (argc > 1)
    {
        projectXML = argv[1];
    }

    HandleProjectXml(projectXML);
}