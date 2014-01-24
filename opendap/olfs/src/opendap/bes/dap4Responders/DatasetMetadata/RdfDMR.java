/*
 * /////////////////////////////////////////////////////////////////////////////
 * // This file is part of the "Hyrax Data Server" project.
 * //
 * //
 * // Copyright (c) 2013 OPeNDAP, Inc.
 * // Author: Nathan David Potter  <ndp@opendap.org>
 * //
 * // This library is free software; you can redistribute it and/or
 * // modify it under the terms of the GNU Lesser General Public
 * // License as published by the Free Software Foundation; either
 * // version 2.1 of the License, or (at your option) any later version.
 * //
 * // This library is distributed in the hope that it will be useful,
 * // but WITHOUT ANY WARRANTY; without even the implied warranty of
 * // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * // Lesser General Public License for more details.
 * //
 * // You should have received a copy of the GNU Lesser General Public
 * // License along with this library; if not, write to the Free Software
 * // Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * //
 * // You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
 * /////////////////////////////////////////////////////////////////////////////
 */

package opendap.bes.dap4Responders.DatasetMetadata;

import opendap.bes.BESError;
import opendap.bes.Version;
import opendap.bes.dap4Responders.Dap4Responder;
import opendap.bes.dap4Responders.MediaType;
import opendap.bes.dapResponders.BesApi;
import opendap.coreServlet.ReqInfo;
import opendap.xml.Transformer;
import org.jdom.Document;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
import org.slf4j.Logger;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.transform.stream.StreamSource;
import java.io.ByteArrayInputStream;

/**
 * Created by IntelliJ IDEA.
 * User: ndp
 * Date: 9/5/12
 * Time: 7:34 PM
 * To change this template use File | Settings | File Templates.
 */
public class RdfDMR extends Dap4Responder {


    private Logger log;
    private static String defaultRequestSuffix = ".rdf";



    public RdfDMR(String sysPath, BesApi besApi) {
        this(sysPath, null, defaultRequestSuffix, besApi);
    }

    public RdfDMR(String sysPath, String pathPrefix, BesApi besApi) {
        this(sysPath, pathPrefix, defaultRequestSuffix, besApi);
    }

    public RdfDMR(String sysPath, String pathPrefix, String requestSuffixRegex, BesApi besApi) {
        super(sysPath, pathPrefix, requestSuffixRegex, besApi);
        log = org.slf4j.LoggerFactory.getLogger(this.getClass());

        setServiceRoleId("http://services.opendap.org/dap4/dataset-metadata");
        setServiceTitle("RDF representation of the DMR");
        setServiceDescription("RDF representation of the Dataset Metadata Response document.");
        setServiceDescriptionLink("http://docs.opendap.org/index.php/DAP4_Web_Services#DAP4:_Dataset_Service_-_The_metadata");

        setNormativeMediaType(new MediaType("application","rdf+xml", getRequestSuffix()));

        log.debug("Using RequestSuffix:              '{}'", getRequestSuffix());
        log.debug("Using CombinedRequestSuffixRegex: '{}'", getCombinedRequestSuffixRegex());

    }


    public boolean isDataResponder(){ return false; }
    public boolean isMetadataResponder(){ return true; }






    public void sendNormativeRepresentation(HttpServletRequest request, HttpServletResponse response) throws Exception {

        String context = request.getContextPath();
        String requestedResourceId = ReqInfo.getLocalUrl(request);
        String xmlBase = getXmlBase(request);

        String resourceID = getResourceId(requestedResourceId, false);
        String constraintExpression = ReqInfo.getConstraintExpression(request);


        BesApi besApi = getBesApi();

        log.debug("sendNormativeRepresentation() - Sending {} for dataset: {}",getServiceTitle(),resourceID);

        response.setContentType(getNormativeMediaType().getMimeType());
        Version.setOpendapMimeHeaders(request, response, besApi);
        response.setHeader("Content-Description", getNormativeMediaType().getMimeType());
        // Commented because of a bug in the OPeNDAP C++ stuff...
        //response.setHeader("Content-Encoding", "plain");


        XMLOutputter xmlo = new XMLOutputter(Format.getPrettyFormat());

        String xdap_accept = "3.2";

        Document ddx = new Document();


        if(!besApi.getDMRDocument(resourceID,constraintExpression,xdap_accept,xmlBase,ddx)){
            BESError besError = new BESError(xmlo.outputString(ddx));
            besError.sendErrorResponse(_systemPath, context, response);
            log.error("sendNormativeRepresentation() - Encountered a BESError:\n" + xmlo.outputString(ddx));
            return;

        }

        ddx.getRootElement().setAttribute("dataset_id",resourceID);

        log.debug(xmlo.outputString(ddx));


        ServletOutputStream os = response.getOutputStream();
        StreamSource ddxStreamSource  = new StreamSource(new ByteArrayInputStream(xmlo.outputString(ddx).getBytes()));

        /*
         Because we are going to daisy chain the XSLT's we have to be careful here!
         */

        // Make the first Transform
        String addRdfId2DapTransformFileName = _systemPath + "/xsl/addRdfId2Dap3.2.xsl";
        Transformer addRdfId2DdxTransform = new Transformer(addRdfId2DapTransformFileName);

        // Grab it's Processor object. All of the XSLT's in the chain must be built
        // using the same Processor
        net.sf.saxon.s9api.Processor proc = addRdfId2DdxTransform.getProcessor();

        // Make the 2nd Transform using the Processor from the first.
        String xml2rdfFileName = _systemPath + "/xsl/anyXml2Rdf.xsl";
        Transformer xml2rdf = new Transformer(proc, xml2rdfFileName);


        // set the destination of the 1st transform to be the 2nd transform
        addRdfId2DdxTransform.setDestination(xml2rdf);

        // Set the destination of the 2nd transform to be the response OutputStream
        xml2rdf.setOutputStream(os);

        // Set the response headers

        String accepts = request.getHeader("Accepts");

        if(accepts!=null && accepts.equalsIgnoreCase("application/rdf+xml"))
            response.setContentType("application/rdf+xml");
        else
            response.setContentType("text/xml");

        Version.setOpendapMimeHeaders(request,response,besApi);
        response.setHeader("Content-Description", getNormativeMediaType().getMimeType());


        // run the 1st transform. This will send the result through the 2nd transform and
        // the result of the 2nd transform will then be sent out the response OutputStream


        try {
            addRdfId2DdxTransform.transform(ddxStreamSource);
        } catch (Exception e) {
            sendRdfErrorResponse(e, resourceID, context, response);
            log.error(e.getMessage());
        }


        log.info("Sent {}.",getServiceTitle());

    }


    public void sendRdfErrorResponse(Exception e, String dataSource, String docsService, HttpServletResponse response) throws Exception {

        String errorMessage =
                        "<p align=\"center\">I'm sorry.</p>\n" +
                        "<p align=\"center\">You requested the RDF representation of the metadata for the dataset:</p>\n" +
                        "<p align=\"center\" class=\"bold\">"+dataSource+" </p>\n" +
                        "<p align=\"center\">The server attempted to transform the metadata in the dataset, " +
                                "represented as a DDX document, into an RDF representation.</p>\n" +
                        "<p align=\"center\">The transform failed, and returned this specific error message:</p>\n" +
                        "<p align=\"center\" class=\"bold\">" + e.getMessage() + "</p>\n";


        sendHttpErrorResponse(500, errorMessage, docsService, response);

    }



}
