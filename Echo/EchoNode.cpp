#include <fstream>

#include <PolySyncRuntimeInfo.hpp>

#include "EchoNode.hpp"
#include "ApplicationInputHandler.hpp"


void PolySyncEcho::initStateEvent()
{ 
    _applicationStartTime = polysync::getTimestamp();

    if( _inputHandler.messageTypesWereFiltered() )
    {
        registerFilteredMessages();
    }
    else if( _inputHandler.activeTypesWereRequested() )
    {
        // do nothing
    }
    else
    {
        registerListenerToAllMessageTypes();
    }
}


void PolySyncEcho::okStateEvent()
{
    if( _inputHandler.durationWasSpecified() )
    {
        if( durationReached() )
        {
            std::cout << std::endl << std::endl << std::endl
                 << "Disconnecting PolySync successfully:"
                 << std::endl
                 << "The time you've specified with -t option argument has "
                 << "expired."
                 << std::endl << std::endl << std::endl;

            disconnectPolySync();
        }
    }

    if( _inputHandler.activeTypesWereRequested() )
    {
        printActiveMessageTypes();
    }

    polysync::sleepMicro( SecondsToMicro );
}


void PolySyncEcho::registerFilteredMessages()
{
    for( auto messageName : _inputHandler.getFilteredMessageNames() )
    {
        tryCatchRegisterAMessageListener( messageName );
    }
}


void PolySyncEcho::tryCatchRegisterAMessageListener( std::string messageName )
{
    try
    {
        registerListener( getMessageTypeByName ( messageName ) );
    }
    catch ( ... )
    {
        std::cout << std::endl
             << "Please enter a valid PS message type:"
             << std::endl << std::endl;

        printAvailableMessage( getAvailableMessageNames() );

        disconnectPolySync();
    }
}


void PolySyncEcho::messageEvent( std::shared_ptr< polysync::Message > message )
{
    if( ( message->getSourceGuid() == getGuid() ) &&
        _inputHandler.ignoreSelfWasRequested()  )
    {
        return;
    }

    if( _inputHandler.fileWasSpecified() )
    {
        printToFile( message );
    }

    printMessage( message );
}


void PolySyncEcho::printToFile(
        std::shared_ptr < polysync:: Message > message )
{
    std::ofstream openUserFile;

    openUserFile.open( _inputHandler.getFileName(), std::ios::app );

    if( _inputHandler.headersWereRequested() )
    {
        message->printHeader( openUserFile );
    }
    else
    {
        message->print( openUserFile );
    }

    openUserFile.close();
}


void PolySyncEcho::printMessage(
        std::shared_ptr < polysync:: Message > message ) const
{
    if( _inputHandler.headersWereRequested() )
    {
        message->printHeader();
    }
    else
    {
        message->print();
    }
}


bool PolySyncEcho::optionsParse( const int argc, char * argv[] )
{
    return _inputHandler.optionsParse( argc, argv );
}


bool PolySyncEcho::helpWasRequested( ) const
{
    return _inputHandler.helpWasRequested();
}


std::vector< std::string > PolySyncEcho::getAvailableMessageNames() const
{
    std::vector< std::string > messageNames;

    for( auto index = 1U;
         index < getAvailableMessageCount() + 1;
         ++index )
    {
        messageNames.emplace_back( getMessageNameByType( index ) );
    }

    return messageNames;
}


void PolySyncEcho::printAvailableMessage(
        const std::vector< std::string > & messageTypeStrings ) const
{
    for( auto messageTypeString : messageTypeStrings )
    {
        std::cout << "    " << messageTypeString << std::endl;
    }
}


void PolySyncEcho::printEchoHelpPage() const
{
    echoHelp.printEchoHelp();
}


void PolySyncEcho::printActiveMessageTypes() const
{
    polysync::RuntimeInfo info( *this );

    auto typeInfoList = info.getDiscoveredMessageTypes();

    std::cout << std::endl << std::endl
              << "Active PolySync Message Types: "
              << std::endl;

    for( auto typeInfo : typeInfoList )
    {
        if( typeInfo.getMessageType() != PSYNC_MSG_TYPE_INVALID )
        {
            std::cout << "    "
                      << typeInfo.getTypeName()
                      << std::endl;
        }
    }
}


bool PolySyncEcho::durationReached() const
{
    return ( polysync::getTimestamp() - _applicationStartTime ) >= getDuration();
}


ps_timestamp PolySyncEcho::getDuration() const
{
    return _inputHandler.getUserSpecifiedDuration() * SecondsToMicro;
}
