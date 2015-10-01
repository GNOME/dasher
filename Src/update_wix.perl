#
#  update_wix.perl updates the alphabet, training and color entries in the WIX installer
#  files.
#
#  Ron Bessems <ron.b@promixis.com>
#

use XML::LibXML;
use XML::LibXML::PrettyPrint;

sub parse_makefile {

    my $filename = @_[0];
    my $regexp = @_[1];
    my @alphabets;

    open(my $fh, '<:encoding(UTF-8)', $filename) or die "Could not open file '$filename' $!";

    while (my $row = <$fh>) {	  
        my @matches = $row =~ @_[1];
        if ( scalar @matches > 0 ) {		
            push @alphabets, @matches[0];
        }
    }	

    close $fh;

    return @alphabets;

}

#Parses Makefile.am and extras the files it lists.
sub parse_alphabet_makefile {
    return parse_makefile( @_[0],  qr/(alphabet\.(:?\w|.)*\.xml)/);
}

#Parses Makefile.am and extras the files it lists.
sub parse_training_makefile {
    return parse_makefile( @_[0],  qr/(training_(:?\w|.)*\.txt)/);	
}

#Parses Makefile.am and extras the files it lists.
sub parse_colours_makefile {
    return parse_makefile( @_[0],  qr/(colour(:?\w|.)*\.xml)/);	
}


# Finds the node with specified xpath, removes it's subnodes and then adds the files as children.
sub update_nodes {
    my $xc = @_[0];
    my $xpath = @_[1];
    my $prefix = @_[2];
    my $files = @_[3];

    my @nodes = $xc->findnodes($xpath);
    if ( scalar @nodes != 1 ) {
        die "Could not find $xpath Node.";
    }

    my $node = @nodes[0];	

    $node->removeChildNodes();	
    for my $file (@$files) {
        my $source = "$prefix$file";
        my $fileNode = $node->addNewChild( '', 'File' );

        $fileNode->setAttribute('Id', $file );
        $fileNode->setAttribute('Name', $file );
        $fileNode->setAttribute('DiskId', '1' );
        $fileNode->setAttribute('Source', $source);				
    }		
	
}

# Takes the source XML and replaces the alphabets, training and colours.
sub parse_xml {

    my $filename = @_[0];
    my $alphabets = @_[1];
    my $training = @_[2];
    my $colours = @_[3];

    my $parser = XML::LibXML->new;	
    my $dom = $parser->parse_file($filename) or die("Could not load $filename");

    my $xc = XML::LibXML::XPathContext->new($dom);
    $xc->registerNs('Wix', 'http://schemas.microsoft.com/wix/2006/wi');

    update_nodes($xc, q {//Wix:Component[@Id="Alphabets"]}, "..\\..\\Data\\alphabets\\", $alphabets);
    update_nodes($xc, q {//Wix:Component[@Id="Training"]}, "..\\..\\Data\\training\\", $training);
    update_nodes($xc, q {//Wix:Component[@Id="Colours"]}, "..\\..\\Data\\colours\\", $colours);

    open my $out_fh, '>', $filename;
    print {$out_fh} XML::LibXML::PrettyPrint->pretty_print($dom);
    close $out_fh;
}

my @alphabets = parse_alphabet_makefile("../Data/alphabets/Makefile.am");
my @training = parse_training_makefile("../Data/training/Makefile.am");
my @colours = parse_colours_makefile("../Data/colours/Makefile.am");

print("Found " . scalar @alphabets . " alphabets\n");
print("Found " . scalar @training . " training files\n");
print("Found " . scalar @colours . " colours\n");

my @files = ( "Installer/Dasher.wxs", "InstallerTobii/InstallerTobii.wxs", "InstallerW2K/InstallerW2K.wxs");

for my $file (@files) {
    parse_xml ( $file, \@alphabets, \@training, \@colours);
}
